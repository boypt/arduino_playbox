#include <DHT.h>
#include <Wire.h>
#include <LedControl.h>
#include <RTClib.h>


// single character message tags
#define TIME_HEADER   'T'   // Header tag for serial time sync message
#define FORMAT_HEADER 'F'   // Header tag indicating a date format message
#define FORMAT_SHORT  's'   // short month and day strings
#define FORMAT_LONG   'l'   // (lower case l) long month and day strings

#define TIME_REQUEST  7     // ASCII bell character requests a time sync message 

#define dht_dpin A0 //no ; here. Set equal to channel sensor is on

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

RTC_DS1307 rtc;


//
LedControl lc = LedControl(12, 11, 10, 2);


byte font[10][8] = {
  {B00000000, B00000111, B00000101, B00000101, B00000101, B00000101, B00000101, B00000111},
  {B00000000, B00000010, B00000110, B00000010, B00000010, B00000010, B00000010, B00000111},
  {B00000000, B00000111, B00000001, B00000001, B00000111, B00000100, B00000100, B00000111},
  {B00000000, B00000111, B00000001, B00000001, B00000111, B00000001, B00000001, B00000111},
  {B00000000, B00000101, B00000101, B00000101, B00000111, B00000001, B00000001, B00000001},
  {B00000000, B00000111, B00000100, B00000100, B00000111, B00000001, B00000001, B00000111},
  {B00000000, B00000111, B00000100, B00000100, B00000111, B00000101, B00000101, B00000111},
  {B00000000, B00000111, B00000001, B00000001, B00000001, B00000001, B00000001, B00000001},
  {B00000000, B00000111, B00000101, B00000101, B00000111, B00000101, B00000101, B00000111},
  {B00000000, B00000111, B00000101, B00000101, B00000111, B00000001, B00000001, B00000111}
};


void setup()  {
  Serial.begin(9600);
  while (!Serial) ; // Needed for Leonardo only
  rtc.begin();
  dht.begin();

//------------------
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  /*
    The MAX72XX is in power-saving mode on startup,
    we have to do a wakeup call
  */
  lc.shutdown(0, false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0, 1);
  /* and clear the display */
  lc.clearDisplay(0);

  lc.shutdown(1, false);
  lc.setIntensity(1, 1);
  lc.clearDisplay(1);

}

void loop() {

/*
  if (Serial.available() > 1) { // wait for at least two characters
    char c = Serial.read();
    if ( c == TIME_HEADER) {
      processSyncMessage();
    }
    else if ( c == FORMAT_HEADER) {
      processFormatMessage();
    }
  }
  if (timeStatus() != timeNotSet) {
    digitalClockDisplay();
  }
  */
    DateTime now = rtc.now();
    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);

    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    
    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");

  writeNumberOnMatrix(1, now.hour(), 1, - 1);
  writeNumberOnMatrix(0, now.minute(), 0, 0);
  //Serial.println(num, DEC);


float h = dht.readHumidity();
float t = dht.readTemperature();
if (isnan(t) || isnan(h)) {
  Serial.println("Failed to read from DHT");
  }
else {
  Serial.println("Temp=");
  Serial.println(t);
  Serial.println(" *C");
  Serial.println("Humidity=");
  Serial.println(h);
  Serial.println("% ");
 }


        
  delay(1000);
}


/*
void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013 - paul, perhaps we define in time.h?

  pctime = Serial.parseInt();
  if ( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
    setTime(pctime); // Sync Arduino clock to the time received on the serial port
  }
}

*/


void writeNumberOnMatrix(byte addr, byte number, int offset_x, int offset_y) {

  byte tens_digit;
  byte unit_digit;

  if (number > 99 ) {
    number = number - 99;
  }

  if (number > 9) {
    unit_digit = number % 10;
    tens_digit = (number - unit_digit) / 10;
  } else {
    unit_digit = number;
    tens_digit = 0;
  }

  byte r;
  for (char i = 0; i < 8; i++ ) {
    r = (byte)font[tens_digit][i] << 4;
    r ^= font[unit_digit][i];
    lc.setRow(addr, i + offset_y, r << offset_x);
  }
}


