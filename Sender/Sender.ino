

/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/

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

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x3C, 0xE9, 0x0E, 0x88, 0x87, 0xC0};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  byte ss;
  byte mi;
  byte hh;
  byte wd;
  byte dm;
  byte mo;
  byte yy;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  Wire1.begin();
  Wire1.setPins(SDA1, SCL1);

  pixels.begin();

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
}

void loop() {
  // Set values to send
  grabTime();

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  // set the first pixel #0 to red
  pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  // and write the data
  pixels.show();

  delay(500);

  // turn off the pixel
  pixels.clear();
  pixels.show();

  delay(500);
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
    myData.ss = bcdToDec(Wire1.read()); // get seconds
    myData.mi = bcdToDec(Wire1.read()); // get minutes
    myData.hh = bcdToDec(Wire1.read()); // get hours
    myData.wd = bcdToDec(Wire1.read()); // get day of week
    myData.dm = bcdToDec(Wire1.read()); // get day of month
    myData.mo = bcdToDec(Wire1.read()); // get month
    myData.yy = bcdToDec(Wire1.read()); // get year
    // indicate that we successfully got the time
  }
}

byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}
