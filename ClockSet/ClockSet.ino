#include <Wire.h>
extern TwoWire Wire1;
// variables for storing the time
//   second  minute  hour    weekday  date    month   year
byte ss=0,   mi=0,   hh=0,   wd=6,    dm=1,   mo=1,   yy=0;

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Wire1.begin();
  Wire1.setPins(SDA1, SCL1);

  // wait to see if a reply is available
  delay(1000);

  Wire1.beginTransmission(0x68); // address DS3231
  Wire1.write(0x0E); // select register
  Wire1.write(0b00011100); // write register bitmap, bit 7 is /EOSC
  Wire1.endTransmission();

  setDS3232time(0, 10, 8, 7, 18, 3, 23);   // set DS3232 seconds, minutes, hours, day of week, date, month, year
}


void loop()
{

}

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val / 10 * 16) + (val % 10) );
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}

void setDS3232time(byte ss, byte mi, byte hh, byte dw, byte dm, byte mo, byte yy)
// sets time and date data to DS3232
{
  Wire1.beginTransmission(0x68);
  Wire1.write(0x00); // sends 00h - seconds register
  Wire1.write(decToBcd(ss));     // set seconds
  Wire1.write(decToBcd(mi));     // set minutes
  Wire1.write(decToBcd(hh));     // set hours
  Wire1.write(decToBcd(dw));    // set day of week (1=Sunday, 7=Saturday)
  Wire1.write(decToBcd(dm));    // set date (1~31)
  Wire1.write(decToBcd(mo));      // set month
  Wire1.write(decToBcd(yy));       // set year (0~99)
  Wire1.endTransmission();
}
