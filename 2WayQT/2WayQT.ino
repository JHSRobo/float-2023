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
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_GFX.h>
#include <Fonts/TomThumb.h> // A tiny 3x5 font incl. w/GFX

// Defines pin a3 on the QT PY as the signal line
#define PIN A3
// Init the neomatrix object with the number of leds and pin
Adafruit_NeoMatrix matrix(5, 5, PIN,
                          NEO_MATRIX_TOP  + NEO_MATRIX_RIGHT +
                          NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE,
                          NEO_GRB         + NEO_KHZ800);

// Defines message for neopixel
const char message[] = "JESUIT";
// Creates colors to be called when float does profiles
const uint16_t colors[] = {matrix.Color(255, 0, 0), matrix.Color(154, 205, 50),
                           matrix.Color(21, 32, 166), matrix.Color(255, 233, 0)
                          };
uint16_t message_width;

// Starts the I2C communication
extern TwoWire Wire1;

// REPLACE WITH THE MAC Address of your receiver
uint8_t broadcastAddress[] = {0x7C, 0xDF, 0xA1, 0x95, 0x51, 0x88};

// Define variables to store readings to be sent
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
bool descending;
bool ascending;

// Variable to store if sending data was successful
String success;
byte wait;

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
  bool _descending;
  bool _ascending;
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
  descending = incomingReadings._descending;
  ascending = incomingReadings._ascending;
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  Wire1.begin();
  Wire1.setPins(SDA1, SCL1);

  // Starts the matrix, sets brightness, font and wrapping
  matrix.begin();
  matrix.setBrightness(5);
  matrix.setFont(&TomThumb);
  matrix.setTextWrap(false);

  matrix.setTextColor(colors[0]); // Start with first color in list

  // Creates the bounds so the text can scroll off a screen
  int16_t  d1;
  uint16_t d2;
  matrix.getTextBounds(message, 0, 0, &d1, &d1, &message_width, &d2);

  // Turns the MISO and SCK pin into GPIO pins
  pinMode(37, OUTPUT);
  pinMode(36, OUTPUT);

  // Init motor value
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

int x = matrix.width();  // Start with message off right edge
int y = matrix.height(); // With custom fonts, y is the baseline, not top
int colornum = 0;

void loop() {
  wait = ss;
  grabTime();
  if (ss != wait) {
    motor++;
  }
  Serial.println(ss);
  myData._ss = ss;
  myData._mi = mi;
  myData._hh = hh;
  myData._wd = wd;
  myData._dm = dm;
  myData._mo = mo;
  myData._yy = yy;
  myData._teamnum = 6;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  if (button == false) {
    // If button false stop the motor and turn JESUIT red
    motor = 0;
    digitalWrite(37, LOW);
    digitalWrite(36, LOW);
    colornum = 0;
  } else {
    // Makes the plunger go up and the float down, turns JESUIT green
    if (motor < 18 && motor > 0) {
      digitalWrite(37, HIGH);
      digitalWrite(36, LOW);
      colornum = 1;
    }
    else if (motor > 38  && motor < 56) {
      // Makes the plunger go down and the float up, turns JESUIT blue
      digitalWrite(37, LOW);
      digitalWrite(36, HIGH);
      colornum = 2;
    }
    else if (motor > 66) {
      // Resets motor so that the float begins another profile
      motor = 0;

    } else {
      //Stops motor so that there is a delay and turns JESUIT gold
      digitalWrite(37, LOW);
      digitalWrite(36, LOW);
      colornum = 3;
    }
  }

  moveMotor();
  Serial.println(motor);

  // Makes the Neogrid blank
  matrix.fillScreen(0);
  // Sets the cursor
  matrix.setCursor(x, y);
  // Prints the message
  matrix.print(message);
  // Turns on the lights
  matrix.show();

  // Moves the text across the screen
  if (--x < -message_width) {
    x = matrix.width();
  }

  // Sets the color if it changes while in 'motor' mode
  matrix.setTextColor(colors[colornum]);

  delay(100);
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
    // indicate that we successfully got the time
  }
}

void moveMotor() {
  if (descending == true) {
    colornum = 1;
    digitalWrite(37, HIGH);
    digitalWrite(36, LOW);
  }

  if (ascending == true) {
    colornum = 2;
    digitalWrite(37, LOW);
    digitalWrite(36, HIGH);
  }
}

// Turns bytes from wire1 to decimals
byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}
