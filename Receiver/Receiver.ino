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

// Structure example to receive data
// Must match the sender structure
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

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print ("\'");
  Serial.print(myData.yy, DEC); Serial.print("-");
  Serial.print(myData.mo, DEC); Serial.print("-");
  Serial.print(myData.dm, DEC); Serial.print("(");
  switch (myData.wd) {
    case 1: Serial.print("Mon"); break;
    case 2: Serial.print("Tue"); break;
    case 3: Serial.print("Wed"); break;
    case 4: Serial.print("Thu"); break;
    case 5: Serial.print("Fri"); break;
    case 6: Serial.print("Sat"); break;
    case 7: Serial.print("Sun"); break;
    default: Serial.print("Bad");
  }
  Serial.print(") ");
  Serial.print(myData.hh, DEC); Serial.print(":");
  Serial.print(myData.mi, DEC); Serial.print(":");
  Serial.print(myData.ss, DEC); Serial.println("");
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {

}
