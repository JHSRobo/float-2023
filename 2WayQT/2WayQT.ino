/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-two-way-communication-esp32/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

extern TwoWire Wire1;
Adafruit_NeoPixel pixels(1, PIN_NEOPIXEL);

// REPLACE WITH THE MAC Address of your receiver
uint8_t broadcastAddress[] = {0x7C, 0xDF, 0xA1, 0x95, 0x51, 0x88};

// Define variables to store BME280 readings to be sent
byte ss;
byte mi;
byte hh;
byte wd;
byte dm;
byte mo;
byte yy;
int teamnum;
int motor;

// Define variables to store incoming readings
bool button;

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
  byte _ss;
  byte _mi;
  byte _hh;
  byte _wd;
  byte _dm;
  byte _mo;
  byte _yy;
  bool _button;
  int _teamnum;
} struct_message;

// Create a struct_message called BME280Readings to hold sensor readings
struct_message myData;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status == 0) {
    success = "Delivery Success :)";
  }
  else {
    success = "Delivery Fail :(";
  }

}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  button = incomingReadings._button;
  Serial.println(incomingReadings._button);
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  Wire1.begin();
  Wire1.setPins(SDA1, SCL1);

  pixels.begin();

  pinMode(37, OUTPUT);
  pinMode(36, OUTPUT);

  motor = 0;

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  grabTime();
  myData._ss = ss;
  myData._mi = mi;
  myData._hh = hh;
  myData._wd = wd;
  myData._dm = dm;
  myData._mo = mo;
  myData._yy = yy;
  myData._teamnum = 12;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

  if (button == false) {
    pixels.setPixelColor(0, pixels.Color(0, 0, 255));
    while (motor < 90) {
      if (motor < 35 && motor > 0) {
        digitalWrite(37, HIGH);
        digitalWrite(36, LOW);
      }
      if (motor > 45  && motor < 80) {
        digitalWrite(37, LOW);
        digitalWrite(36, HIGH);
      }
    }
    motor = 0;
  } else {
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    if (motor < 35 && motor > 0) {
      digitalWrite(37, HIGH);
      digitalWrite(36, LOW);
    }
    if (motor > 45  && motor < 80) {
      digitalWrite(37, LOW);
      digitalWrite(36, HIGH);
    }
    if (motor > 90) {
      motor = 0;
    }
  }


  // and write the data
  pixels.show();

  delay(250);

  // turn off the pixel
  pixels.clear();
  pixels.show();

  delay(250);
}
void grabTime() {
  // get time from the RTC and put it in global variables

  // send request to receive data starting at register 0
  Wire1.beginTransmission(0x68); // 0x68 is DS3231 device address
  Wire1.write((byte)0); // start at register 0
  Wire1.endTransmission();
  Wire1.requestFrom(0x68, 7); // request seven bytes (ss, mi, hh, wd, dd, mo, yy)
  // check for a reply from the RTC, and use it if we can
  if (Wire1.available() >= 7) {
    // if we're here, we got a reply and it is long enough
    // so now we read the time
    ss = bcdToDec(Wire1.read()); // get seconds
    mi = bcdToDec(Wire1.read()); // get minutes
    hh = bcdToDec(Wire1.read()); // get hours
    wd = bcdToDec(Wire1.read()); // get day of week
    dm = bcdToDec(Wire1.read()); // get day of month
    mo = bcdToDec(Wire1.read()); // get month
    yy = bcdToDec(Wire1.read()); // get year
    motor++;
    // indicate that we successfully got the time
  }
}

byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}
