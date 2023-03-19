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

  ss = bcdToDec(Wire1.read()); // get seconds
  mi = bcdToDec(Wire1.read()); // get minutes
  hh = bcdToDec(Wire1.read()); // get hours
  wd = bcdToDec(Wire1.read()); // get day of week
  dm = bcdToDec(Wire1.read()); // get day of month
  mo = bcdToDec(Wire1.read()); // get month
  yy = bcdToDec(Wire1.read()); // get year

  // wait to see if a reply is available
  delay(1000);
}


void loop()
{
  boolean gotTheTime = grabTime();

  if (gotTheTime) {
    // if we are here, then the time has been successfully read
    // and stored in global variables (ss, mi, hh, wd, dd, mo, yy)
    Serial.print("Got the time: ");
    printTime();
  }
  else {
    // if we are here, then we tried to read the time but couldn't
    Serial.println("Unable to read time from RTC");
  }

  delay(500);
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

boolean grabTime() {
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
    yy = bcdToDec(Wire1.read()); // get year (two digits)
    // indicate that we successfully got the time
    return true;
  }
  else {
    // indicate that we were unable to read the time
    return false;
  }
}

void printTime() {
  // just like it says on the tin
  Serial.print ("\'");
  Serial.print(yy, DEC); Serial.print("-");
  Serial.print(mo, DEC); Serial.print("-");
  Serial.print(dm, DEC); Serial.print("(");
  switch (wd) {
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
  Serial.print(hh, DEC); Serial.print(":");
  Serial.print(mi, DEC); Serial.print(":");
  Serial.print(ss, DEC); Serial.println("");
}
