/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <esp_now.h>
#include <WiFi.h>

uint8_t broadcastAddress[] = {0xD4, 0xF9, 0x8D, 0x71, 0x15, 0x16};

// Define variables to store BME280 readings to be sent
bool button;

// Define variables to store incoming readings
byte ss;
byte mi;
byte hh;
byte wd;
byte dm;
byte mo;
byte yy;
int teamnum;

// Variable to store if sending data was successful
String success;

// Structure example to receive data
// Must match the sender structure
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

// Create a struct_message called myData
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

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  yy = incomingReadings._yy;
  mo = incomingReadings._mo;
  dm = incomingReadings._dm;
  wd = incomingReadings._wd;
  hh = incomingReadings._hh;
  mi = incomingReadings._mi;
  ss = incomingReadings._ss;
  teamnum = incomingReadings._teamnum;

}

const int buttonPin = 18;
const int ledPin =  LED_BUILTIN;
int buttonState = 0;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);

  pinMode(buttonPin, INPUT);

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

  // turn on backlite
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  // turn on the TFT / I2C power supply
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  // initialize TFT
  tft.init(135, 240); // Init ST7789 240x135
  tft.setRotation(3); // Sets the orientation
  tft.fillScreen(ST77XX_BLACK); // Sets background color

  Serial.println(F("Initialized"));
}

void loop() {
  // Calls button function
  ButtonPressed(); 

  // Sets the button data to current
  myData._button = button;

  // Checks if data was sent
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  // Prints check
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

  // Calls display function
  DisplayData();
}

// Displays data on the TFT
void DisplayData() {
  tft.setCursor(10, 10); // Sets the location of the text
  tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK); // Sets the color
  tft.setTextSize(2);
  tft.print("Date: ");
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  // Displays the date, including year, month, day, and day of week
  tft.print(yy, DEC); tft.print("-");
  tft.print(mo, DEC); tft.print("-");
  tft.print(dm, DEC); tft.print("(");
  switch (wd) {
    case 1: tft.print("Mon"); break;
    case 2: tft.print("Tue"); break;
    case 3: tft.print("Wed"); break;
    case 4: tft.print("Thu"); break;
    case 5: tft.print("Fri"); break;
    case 6: tft.print("Sat"); break;
    case 7: tft.print("Sun"); break;
    default: tft.print("Bad");
  }
  tft.println(") ");
  tft.setCursor(10, 40);
  tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
    /* Displays UTC time, including hour, minute, and second, 
  with the last two having functions to display zeros infront */
  tft.print("Time: ");
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.print(hh, DEC); tft.print(":");
  if ( mi > 9) {
    tft.print(mi, DEC); tft.print(":");
  } else {
    tft.print(0); tft.print(mi, DEC); tft.print(":");
  }
  if (ss > 9) {
    tft.print(ss, DEC);  tft.println("");
  } else {
    tft.print(0); tft.print(ss, DEC);  tft.println("");
  }

  tft.setCursor(10, 70);
  tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
  tft.setTextSize(2);
  // Displays the team number
  tft.print("Team Number: ");
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.print(teamnum);

  tft.setCursor(10, 100);
  tft.setTextColor(ST77XX_BLUE, ST77XX_BLACK);
  tft.setTextSize(2);
   // Displays if the button has been pressed, not if the QT PY has recieved the message
  tft.print("Button Pressed: ");
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.print(button);
}

// Function to update button and switch the on and off
void ButtonPressed() {
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
    button = !button;
    delay(500);
  }
}
