#include <LiquidCrystal.h>
#include "TaskScheduler.h"
#include "dht.h"
#include <Wire.h>
#include <LedControl.h>
#include <RTClib.h>

#define TIME_HEADER  "T"   // Header tag for serial time sync message

#define DHTPIN 2
//DHT dht(2, DHT11);  // DHT PIN2 DS -> 2
dht DHT;
RTC_DS1307 rtc;     // SDA-> A4, SCL->A5

LedControl lc = LedControl(12, 11, 10, 2); // DIN->12, CLK->11, CS->10
LiquidCrystal lcd(5, 4, 9, 8, 7, 6); //RS->5, E->4, D4/D5/D6/D7->9/8/7/6



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

  rtc.begin();
  lcd.begin(16, 2);

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
  lc.shutdown(1, false);

  /* Set the brightness to a medium values */
  lc.setIntensity(0, 1);
  lc.setIntensity(1, 1);

  /* and clear the display */
  lc.clearDisplay(0);
  lc.clearDisplay(1);


  Sch.init();

  Sch.addTask(readTime, 0, 250, 1);
  Sch.addTask(writeTime, 0, 500, 1);

  Sch.addTask(readDHT, 0, 5000, 1);
  Sch.addTask(writeTH, 1000, 5000, 1);
  Sch.addTask(blinkLED, 100, 1000, 1);
  Sch.addTask(readSerial, 800, 1000, 1);
  Sch.start();

}

String twoDigit(int8_t val) {
  String z = String(0);
  if (val < 10) {
    return z + String(val, DEC);
  } else if (val > 9) {
    return String(val, DEC);
  }
}

void loop()
{
  Sch.dispatchTasks();
}

DateTime now;
int8_t h, t;

void readTime() {
  now = rtc.now();
}

void readDHT() {
  DHT.read11(DHTPIN);
  h = DHT.humidity;
  t = DHT.temperature >> 1;
}

void writeTime() {
  String datatime = " " + twoDigit(now.month()) + "/" + twoDigit(now.day()) + " " + twoDigit(now.hour()) + ":" + twoDigit(now.minute()) + ":" + twoDigit(now.second());
  lcd.setCursor(0, 0);
  lcd.print(datatime);
  writeNumberOnMatrix(1, now.hour(), 1, - 1);
  writeNumberOnMatrix(0, now.minute(), 0, 0);
}

void writeTH() {
  String temphum;
  temphum = "TMP " + String(t, DEC) + "\xDF" + (t>9?"C HUM ":"C  HUM ") + String(h, DEC) + "%";
  lcd.setCursor(0, 1);
  lcd.print(temphum);
}

boolean blink = false;
void blinkLED() {
  lc.setLed(1, 7, 7, blink);
  lc.setLed(0, 0, 0, !blink);
  blink = !blink;
}

void readSerial() {
  if (Serial.available()) {
    processSyncMessage();
  }
}


void processSyncMessage() {
  uint32_t pctime;
  const uint32_t DEFAULT_TIME = 1357041600; // Jan 1 2013 - paul, perhaps we define in time.h?
  if (Serial.find((char*)"T")) {
    pctime = Serial.parseInt();
    if ( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
      DateTime dt(pctime+(3600*8)); //+8 timezone
      rtc.adjust(dt);
    }
  }
}



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
  for (byte i = 0; i < 8; i++ ) {
    r = font[tens_digit][i] << 4;
    r ^= font[unit_digit][i];
    lc.setRow(addr, i + offset_y, r << offset_x);
  }
}
