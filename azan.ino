#include <Wire.h>
#include "RTClib.h"
#include <Encoder.h>
#include "U8glib.h"
#include <TinyGPS++.h>
#include <TimeLib.h>
#include <DateConvL.h>
#include <AzanTime.h>
#include "DFRobotDFPlayerMini.h"
#include "banner.h"

DFRobotDFPlayerMini player;
DateConvL dateC;
TinyGPSPlus gps;
#define BUSY_PIN 9
#define time_offset 12600
#define VolumeUp 52
#define VolumeDown 53
#define Dfserial Serial2
#define GpsSerial Serial3
#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
#define Serial SerialUSB
#endif
Encoder knob(3, 2);  //encoder connected to pins 2 Yeard 3 (Yeard ground)
//RTC_DS1307 rtc;
RTC_DS3231 rtc;
float lat = 35.572190;  // Latitude  35.572190     35.804159
float lg = 51.640727;   // Longitude 51.640727     51.073654

char dayofweek[7][12] = {
  "1-Shanbeh",
  "2-Shanbeh",
  "3-Shanbeh",
  "4-Shanbeh",
  "5-Shanbeh",
  "Jomeh",
  "Shanbeh"
};
//U8GLIB_T6963_240X128 u8g(8,  9, 10, 11,  4,  5,  6,  7,    14,         15,    17,    18,      16);
// 8Bit Com: D0..D7:    D0, D1, D2, D3, D4, D5, D6, D7, cs=14, c/d(a0)=15, wr=17, rd=18, reset=16
// setup u8g object, please remove comment from one of the following constructor calls
// IMPORTYearT NOTE: The complete list of supported devices is here: http://code.google.com/p/u8glib/wiki/device
//  U8GLIB_ST7920_128X64_1X u8g(13, 11, 10); // SPI Com: SCK = en = 13, MOSI = rw = 11, CS = di = 10
// se orginal code for your LCD!!!!!!
U8GLIB_ST7920_128X64_1X u8g(7, 6, 5, 8);  //Enable, RW, RS, RESET

int X, Y, I, X2, Y2, X3, Y3 = 0;

float latitude, longitude;
float Vinkel = 0;
int Days = 0;
int posicao = 0;
int posicaoh = 0;

int ScreenWith = 128;
int ScreenWithC = 64;
int ScreenHeight = 64;
int ScreenHeightC = 32;
int deics = ScreenWith / 4;
int deigrec = 14;
int deics2 = 0;
int de = 10;
//the variables provide the holding values for the set clock routine
int setyeartemp;
int setmonthtemp;
int setdaytemp;
int setoretemp;
int setDaytemp;
int setminstemp;
char setgpstemp = 0;
int setsecs = 0;
int maxday;  // maximum number of days in the given month
int Displaytemp;
// These variables are for the push button routine
int buttonstate = 0;             //flag to see if the button has been pressed, used internal on the subroutine only
int pushlengthset = 1500;        // value for a long push in mS
int pushlength = pushlengthset;  // set default pushlength
int pushstart = 0;               // sets default push value for the button going low
int pushstop = 0;                // sets the default value for when the button goes back high
int knobval;
int Volume = 20;             // value for the rotation of the knob
boolean buttonflag = false;  // default value for the button flag
#define buton 4
char tmp_string[8];

void Besmelladraw(void) {
  u8g.drawXBMP(0, 0, 128, 64, Besmella);
}
String timeToString(struct Time& time) {
  char str[9];
  sprintf(str, "%02d:%02d:%02d", time.hour, time.minute, time.second);
  return String(str);
}
void printTime(struct Time& time, Print& out) {
  out.println(timeToString(time));
}
// String formatOghat(const String &title, Time &time) {
//   return title + " " + time.hour + ":" + time.minute + ":" + time.second;
//}
void setup(void) {
  pinMode(VolumeUp, INPUT_PULLUP);
  pinMode(VolumeDown, INPUT_PULLUP);
  u8g.firstPage();
  do {
    Besmelladraw();
  } while (u8g.nextPage());

  delay(3000);

  u8g.setFont(u8g_font_6x10);
  pinMode(9, INPUT);
  digitalWrite(9, HIGH);
  // #ifndef ESP8266
  //   while (!Serial); // for Leonardo/Micro/Zero
  // #endif
  Serial.begin(9600);
  Dfserial.begin(9600);
  GpsSerial.begin(9600);
  if (!player.begin(Dfserial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    player.setTimeOut(500);  //Set serial communictaion time out 500ms
    //----Set volume----
    player.volume(Volume);  //Set volume value (0~30).
    //player.volumeUp();    //Volume Up
    // player.volumeDown();  //Volume Down

    //----Set different EQ----
    player.EQ(DFPLAYER_EQ_NORMAL);

    // myDFPlayer.play(1);  //Play the first mp3
  }

  player.play(1);

  if (!rtc.begin()) {
    Serial.print("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    //while (1)
  }
  pinMode(buton, INPUT);      //push button on encoder connected to A0 (Yeard GND)
  digitalWrite(buton, HIGH);  //Pull button high
}

void loop(void) {
  //u8g.setFont(u8g_font_unifont);
  u8g.setFont(u8g_font_6x10);
  DateTime now = rtc.now();
  dateC.ToShamsi(now.year(), now.month(), now.day());

  Days = now.dayOfTheWeek();
  Serialprint(now);
  Oghat today = AzanTime(dateC.global_month, dateC.global_day, lat, lg);
  printOghat(today, Serial);
  AzanAlarm(today, now);
  if (digitalRead(VolumeUp) == 0 && Volume < 30) {
    Volume = Volume + 1;
    player.volume(Volume);
  }
  if (digitalRead(VolumeDown) == 0 && Volume > 0) {
    Volume = Volume - 1;
    player.volume(Volume);
  }
  delay(200);
  u8g.firstPage();
  do {
    draw(today, now);
  } while (u8g.nextPage());
  // rebuild the picture after some delay
  delay(100);

  pushlength = pushlengthset;
  pushlength = getpushlength();
  delay(10);

  if (pushlength < pushlengthset) {
    u8g.firstPage();
    do {
      draw1(today);
    } while (u8g.nextPage());
    // rebuild the picture after some delay
    delay(4000);
  }
  //This runs the setclock routine if the knob is pushed for a long time
  if (pushlength > pushlengthset) {
    Serial.println("long push");
    setyeartemp = now.year();
    setmonthtemp = now.month();
    setdaytemp = now.day();
    setoretemp = now.hour();
    setminstemp = now.minute();
    setDaytemp = now.dayOfTheWeek();
    setclock();
    pushlength = pushlengthset;
  };
}  // end main loop
void Serialprint(DateTime& now) {
  Serial.print(now.hour());
  Serial.print(':');
  Serial.print(now.minute());
  Serial.print(':');
  Serial.print(now.second());
  Serial.print(" - ");
  Serial.print(now.year());
  Serial.print('/');
  Serial.print(now.month());
  Serial.print('/');
  Serial.print(now.day());
  Serial.print(" (");
  Serial.print(dayofweek[Days]);
  Serial.println(") ");
  Serial.print(dateC.global_year);
  Serial.print('/');
  Serial.print(dateC.global_month);
  Serial.print('/');
  Serial.print(dateC.global_day);
  Serial.println();
  Serial.print("lat  = ");
  Serial.print(lat);
  Serial.println();
  Serial.print("lg  = ");
  Serial.print(lg);
  Serial.println();
  Serial.print("latitude =");
  Serial.print(latitude);
  Serial.println();
  Serial.print("longitude =");
  Serial.print(longitude);
  Serial.println();
}

// subroutine to return the length of the button push.
int getpushlength() {
  buttonstate = digitalRead(buton);
  if (buttonstate == LOW && buttonflag == false) {
    pushstart = millis();
    buttonflag = true;
  };
  if (buttonstate == HIGH && buttonflag == true) {
    pushstop = millis();
    pushlength = pushstop - pushstart;
    buttonflag = false;
  };
  Serial.println("_");
  return pushlength;
}
void draw(Oghat& oghat, DateTime& now) {

  knobval = knob.read();
  if (knobval < -1) {  //bit of software de-bounce
    knobval = 0;
    DisplayOn(now, oghat);
    delay(50);
  }
  if (knobval > 1) {
    knobval = 1;
    DisplayTwo();
    delay(50);
  }

  //Displaytemp = Displaytemp + knobval;
  if (knobval == 0) {
    DisplayOn(now, oghat);
  }
  // if (Displaytemp == 1) {
  //   DisplayTwo();
  // }
  // if (Displaytemp == 2) {
  //   DisplayTree();
  // }
}
void DisplayOn(DateTime& now, Oghat& oghat) {
  // graphic commYeards to redraw the complete screen should be placed here
  //  u8g.setFont(u8g_font_6x10);
  //u8g.setFont(u8g_font_osb21);
  AnalogClock();
  AnalogHour(now.hour() - 1, 12.0, 15);    // Omdreinig, Omdreiningstall / omdreining, radius
  AnalogHour(now.minute() - 5, 60.0, 24);  // Omdreinig, Omdreiningstall / omdreining, radius
  //  AnalogHour(Second, 60-deics, 27); // Omdreinig, Omdreiningstall / omdreining, radius
  AnalogHour(now.second() - 6, 60.0, 27);
  // AnalogHour(Second, 60.0, 27);
  u8g.setFont(u8g_font_6x10);
  //u8g.drawStr(2 * deics + deics2, 30, (titleOghat(now, oghat)));
  u8g.setPrintPos(2 * deics + deics2, 30);
  u8g.print(nextOghatTitle(now, oghat));
  u8g.setPrintPos(2 * deics + (deics2 + 10), 40);
  u8g.print(nextOghatTime(now, oghat));
  u8g.setPrintPos(2 * deics + deics2, 50);
  u8g.print(dateC.global_year);
  u8g.print("/");
  if (dateC.global_month < 10) u8g.print("0");
  u8g.print(dateC.global_month);
  u8g.print("/");
  if (dateC.global_day < 10) u8g.print("0");
  u8g.print(dateC.global_day);

  u8g.setPrintPos(2 * deics + deics2, 60);
  u8g.print(dayofweek[Days]);
  //u8g.setPrintPos(2*deics + deics2, 20);
  //u8g.print(Year);

  u8g.drawRBox(64, 2, 62, 21, 2);
  u8g.setColorIndex(0);
  u8g.setFont(u8g_font_fur17);
  if (now.second() % 2 != 1)
    u8g.drawStr(90, 19, ":");
  if (now.hour() < 10) {
    u8g.drawStr(64, 21, "0");
    posicaoh = 77;
  } else posicaoh = 64;
  u8g.setPrintPos(posicaoh, 21);
  u8g.print(now.hour());
  if (now.minute() < 10) {
    u8g.drawStr(98, 21, "0");
    posicao = 112;
  } else posicao = 98;
  u8g.setPrintPos(posicao, 21);
  u8g.print(now.minute());
  u8g.setColorIndex(1);
  u8g.setFont(u8g_font_6x10);
}
void DisplayTwo() {
  u8g.drawRFrame(0, 0, 128, 64, 3);
  u8g.drawRFrame(2, 2, 124, 28, 2);
  u8g.drawRFrame(2, 31, 55, 15, 2);
  u8g.drawRFrame(58, 31, 68, 15, 2);

  getGPS();

  u8g.setPrintPos(2 * deics + deics2 - 1, 42);
  u8g.setFont(u8g_font_6x10);
  if (day() < 10) u8g.print("0");
  u8g.print(day());
  u8g.print("-");
  if (month() < 10) u8g.print("0");
  u8g.print(month());
  u8g.print("-");
  u8g.print(year());

  u8g.setFont(u8g_font_6x10);
  u8g.setPrintPos(4, 60);
  u8g.print("lat=");
  u8g.print(latitude);
  u8g.setPrintPos(64, 60);
  u8g.print("lg=");
  u8g.print(longitude);
  u8g.setPrintPos(4, 42);
  u8g.setFont(u8g_font_6x10);
  u8g.print(dayofweek[Days]);

  u8g.setPrintPos(4, 26);
  u8g.setFont(u8g_font_fub20n);
  if (hour() < 10) u8g.print(" ");
  u8g.print(hour());
  u8g.print(":");
  if (minute() < 10) u8g.print("0");
  u8g.print(minute());
  u8g.print(":");
  if (second() < 10) u8g.print("0");
  u8g.print(second());
  u8g.setFont(u8g_font_6x10);
}
void draw1(Oghat& oghat) {
  u8g.drawStr(30, 10, "Azan TIME ");
  u8g.drawStr(2, 20, " Morning ");
  u8g.drawStr(65, 20, timeToString(oghat.Morning).c_str());
  u8g.drawStr(2, 30, " SunRize ");
  u8g.drawStr(65, 30, timeToString(oghat.SunRise).c_str());
  u8g.drawStr(2, 40, " Noon ");
  u8g.drawStr(65, 40, timeToString(oghat.Noon).c_str());
  u8g.drawStr(2, 50, " Maghrib ");
  u8g.drawStr(65, 50, timeToString(oghat.Maghrib).c_str());
}

//sets the clock
void setclock() {
  u8g.setFont(u8g_font_8x13);
  u8g.firstPage();
  do {
    u8g.drawStr(4, 30, "Set Auto   +>>>");
    u8g.drawStr(4, 50, "Set Manoal <<<-");
  } while (u8g.nextPage());
  while (1) {

    knob.write(0);
    delay(50);
    knobval = knob.read();
    if (knobval < 0) {  //bit of software de-bounce
      knobval = -1;
      delay(50);
    }
    if (knobval > 0) {
      knobval = 1;
      delay(50);
    }
    setgpstemp = setgpstemp + knobval;
    if (setgpstemp == -1) {
      u8g.firstPage();
      do {
        u8g.drawStr(20, 40, "Manoally ");
      } while (u8g.nextPage());
      delay(1000);
      setyear();
      setmonth();
      setday();
      //setDay();
      setore();
      setmins();
      rtc.adjust(DateTime(setyeartemp, setmonthtemp, setdaytemp, setoretemp, setminstemp, setsecs));
      delay(100);
      setgpstemp = 0;
      u8g.setFont(u8g_font_6x10);
      return;
    } else if (knobval == 1) {
      u8g.firstPage();
      do {
        setgpsAll();
      } while (u8g.nextPage());
      delay(3000);
      setgpstemp = 0;
      knob.write(0);
      return;
    }
  }
}
// The following subroutines set the individual clock parameters
int setyear() {
  pushlength = pushlengthset;
  pushlength = getpushlength();
  if (pushlength != pushlengthset) {
    return setyeartemp;
  }
  knob.write(0);
  delay(50);
  knobval = knob.read();
  if (knobval < -1) {  //bit of software de-bounce
    knobval = -1;
    delay(50);
  }
  if (knobval > 1) {
    knobval = 1;
    delay(50);
  }
  setyeartemp = setyeartemp + knobval;
  if (setyeartemp < 2018) {  //Year cYear't be older thYear currently, it's not a time machine.
    setyeartemp = 2018;
  }
  itoa(setyeartemp, tmp_string, 10);
  u8g.firstPage();
  do {
    u8g.drawStr(0, 20, "Set Year");
    u8g.drawStr(25, 40, tmp_string);
  } while (u8g.nextPage());
  setyear();
}

int setmonth() {
  pushlength = pushlengthset;
  pushlength = getpushlength();
  if (pushlength != pushlengthset) {
    return setmonthtemp;
  }

  knob.write(0);
  delay(50);
  knobval = knob.read();
  if (knobval < -1) {
    knobval = -1;
  }
  if (knobval > 1) {
    knobval = 1;
  }
  setmonthtemp = setmonthtemp + knobval;
  if (setmonthtemp < 1) {  // month must be between 1 Yeard 12
    setmonthtemp = 12;
  }
  if (setmonthtemp > 12) {
    setmonthtemp = 1;
  }
  itoa(setmonthtemp, tmp_string, 10);
  u8g.firstPage();
  do {
    u8g.drawStr(0, 20, "Set Month");
    u8g.drawStr(25, 40, tmp_string);
  } while (u8g.nextPage());
  setmonth();
}

int setday() {
  if (setmonthtemp == 4 || setmonthtemp == 5 || setmonthtemp == 9 || setmonthtemp == 11) {  //30 days hath September, April June Yeard November
    maxday = 30;
  } else {
    maxday = 31;  //... all the others have 31
  }
  if (setmonthtemp == 2 && setyeartemp % 4 == 0) {  //... Except February alone, Yeard that has 28 days clear, Yeard 29 in a leap year.
    maxday = 29;
  }
  if (setmonthtemp == 2 && setyeartemp % 4 != 0) {
    maxday = 28;
  }
  pushlength = pushlengthset;
  pushlength = getpushlength();
  if (pushlength != pushlengthset) {
    return setdaytemp;
  }
  knob.write(0);
  delay(50);
  knobval = knob.read();
  if (knobval < -1) {
    knobval = -1;
  }
  if (knobval > 1) {
    knobval = 1;
  }
  setdaytemp = setdaytemp + knobval;
  if (setdaytemp < 1) {
    setdaytemp = 1;
  }
  if (setdaytemp > maxday) {
    setdaytemp = maxday;
  }
  itoa(setdaytemp, tmp_string, 10);
  u8g.firstPage();
  do {
    u8g.drawStr(0, 20, "Set Day");
    u8g.drawStr(25, 40, tmp_string);
  } while (u8g.nextPage());
  setday();
}

int setDay() {
  pushlength = pushlengthset;
  pushlength = getpushlength();
  if (pushlength != pushlengthset) {
    return setDaytemp;
  }
  knob.write(0);
  delay(50);
  knobval = knob.read();
  if (knobval < -1) {
    knobval = -1;
  }
  if (knobval > 1) {
    knobval = 1;
  }
  setDaytemp = setDaytemp + knobval;
  if (setDaytemp < 0) {  // month must be between 0 Yeard 6
    setDaytemp = 0;
  }
  if (setDaytemp > 6) {
    setDaytemp = 6;
  }
  itoa(setDaytemp, tmp_string, 10);
  u8g.firstPage();
  do {
    u8g.drawStr(0, 20, "Set Day of Week");
    u8g.drawStr(25, 40, tmp_string);
    u8g.drawStr(0, 60, dayofweek[setDaytemp]);
  } while (u8g.nextPage());
  setDay();
}

bool operator<(DateTime& time, Time& tod) {
  return (time.hour() < tod.hour) || (time.hour() == tod.hour && time.minute() < tod.minute) || (time.hour() == tod.hour && time.minute() == tod.minute && time.second() < tod.second);
}
String nextOghatTitle(DateTime& now, Oghat& oghat) {
  if (now < oghat.Morning) return "  Morning";
  if (now < oghat.SunRise) return "  SunRise";
  if (now < oghat.Noon) return "  Zuhr";
  if (now < oghat.Maghrib) return "  Maghrib";
  return "";
}

String nextOghatTime(DateTime& now, Oghat& oghat) {
  if (now < oghat.Morning) return timeToString(oghat.Morning);
  if (now < oghat.SunRise) return timeToString(oghat.SunRise);
  if (now < oghat.Noon) return timeToString(oghat.Noon);
  if (now < oghat.Maghrib) return timeToString(oghat.Maghrib);
  return "";
}

// String nextOghatTime(DateTime &time, Oghat &oghat) {
//   if (now < oghat.Morning) return formatOghat("Fajr", oghat.fajr);
//   if (now < oghat.sunRise) return formatOghat("SunRise", oghat.sunRise);
//   if (now < oghat.SunRise) return formatOghat("Zuhr", oghat.zuhr);
//   if (now < oghat.Maghrib) return formatOghat("Maghrib", oghat.maghrib);
//   return "";
// }
// String formatOghat(const String &title, TimeOfDay &time) {
//   return title + " " + time.hour + ":" + time.minute;
// }
void setgpsAll() {
  u8g.firstPage();
  getGPS();
  do {
    if (gps.satellites.isValid()) {
      rtc.adjust(DateTime(year(), month(), day(), hour(), minute(), second()));
      if (gps.location.isValid()) {
        lat = latitude;
        lg = longitude;
      }
      u8g.drawStr(0, 30, "Set GPS All");
    } else {
      u8g.drawStr(0, 30, "No GPS data!");
    }
  } while (u8g.nextPage());
  delay(1000);
}

void getGPS() {
  int gDay, gMonth, gYear;
  int gMinute, gSecond, gHour;
  while (GpsSerial.available() > 0)
    if (gps.encode(GpsSerial.read())) {
      if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
      }
      // get date and time from GPS module
      if (gps.date.isUpdated() && gps.time.isUpdated()) {
        gDay = gps.date.day();
        gMonth = gps.date.month();
        gYear = gps.date.year();
        gMinute = gps.time.minute();
        gSecond = gps.time.second();
        gHour = gps.time.hour();
        // set current UTC time
        setTime(gHour, gMinute, gSecond, gDay, gMonth, gYear);
        // add the offset to get local time
        adjustTime(time_offset);

        // return true;
      }
    }
  //return false;
}
int setore() {
  pushlength = pushlengthset;
  pushlength = getpushlength();
  if (pushlength != pushlengthset) {
    return setoretemp;
  }
  knob.write(0);
  delay(50);
  knobval = knob.read();
  if (knobval < -1) {
    knobval = -1;
    delay(50);
  }
  if (knobval > 1) {
    knobval = 1;
    delay(50);
  }
  setoretemp = setoretemp + knobval;
  if (setoretemp < 0) {
    setoretemp = 23;
  }
  if (setoretemp > 23) {
    setoretemp = 0;
  }
  itoa(setoretemp, tmp_string, 10);
  u8g.firstPage();
  do {
    u8g.drawStr(0, 20, "Set Hour");
    u8g.drawStr(25, 40, tmp_string);
  } while (u8g.nextPage());
  setore();
}

int setmins() {
  pushlength = pushlengthset;
  pushlength = getpushlength();
  if (pushlength != pushlengthset) {
    return setminstemp;
  }
  knob.write(0);
  delay(50);
  knobval = knob.read();
  if (knobval < -1) {
    knobval = -1;
    delay(50);
  }
  if (knobval > 1) {
    knobval = 1;
    delay(50);
  }
  setminstemp = setminstemp + knobval;
  if (setminstemp < 0) {
    setminstemp = 59;
  }
  if (setminstemp > 59) {
    setminstemp = 0;
  }
  itoa(setminstemp, tmp_string, 10);
  u8g.firstPage();
  do {
    u8g.drawStr(0, 20, "Set Minutes");
    u8g.drawStr(25, 40, tmp_string);
  } while (u8g.nextPage());
  setmins();
}
void AnalogHour(float Omdreining, float forhold, int Radius) {
  Vinkel = Omdreining * 2.0 * 3.1415 / forhold - 1, 5707;  // 12 Hour blir til 2Pi
  X2 = ScreenWithC - deics + Radius * cos(Vinkel);
  Y2 = ScreenHeightC + Radius * sin(Vinkel);
  u8g.drawLine(ScreenWithC - deics, ScreenHeightC, X2, Y2);
}

void AnalogClock() {  // draw clock background
  u8g.drawCircle(ScreenWithC - deics, ScreenHeightC, 1);
  u8g.drawStr(55 - deics, 4 + deigrec, "12");
  u8g.drawStr(83 - deics, 25 + deigrec, "3");
  u8g.drawStr(60 - deics, 45 + deigrec, "6");
  u8g.drawStr(39 - deics, 25 + deigrec, "9");

  for (int TimeStrek = 0; TimeStrek < 12; TimeStrek++) {  // draw time lines
    Vinkel = TimeStrek / 12.0 * 2 * 3.1415;
    X2 = ScreenWithC - deics + 30 * cos(Vinkel);
    Y2 = ScreenHeightC + 30 * sin(Vinkel);
    X3 = ScreenWithC - deics + 28 * cos(Vinkel);
    Y3 = ScreenHeightC + 28 * sin(Vinkel);
    u8g.drawLine(X2, Y2, X3, Y3);
  }
}
bool AzanCheck(struct Oghat oghat, DateTime& now) {
  return (
    (oghat.Morning.hour == now.hour() && oghat.Morning.minute == now.minute() && oghat.Morning.second == now.second()) || (oghat.Noon.hour == now.hour() && oghat.Noon.minute == now.minute() && oghat.Noon.second == now.second()) || (oghat.Maghrib.hour == now.hour() && oghat.Maghrib.minute == now.minute() && oghat.Maghrib.second == now.second()));
}
bool AzanCheckPlay(struct Oghat oghat, DateTime& now) {
  return (
    (oghat.Morning.hour == now.hour() && oghat.Morning.minute == now.minute() && oghat.Morning.second < now.second()) || (oghat.Noon.hour == now.hour() && oghat.Noon.minute == now.minute() && oghat.Noon.second < now.second()) || (oghat.Maghrib.hour == now.hour() && oghat.Maghrib.minute == now.minute() && oghat.Maghrib.second < now.second()));
}

void AzanAlarm(struct Oghat oghat, DateTime& now) {
  if (AzanCheck(oghat, now)) {
     player.play(7); 
     delay(100) ;
     player.play(7); 
     
     Serial.println("azan\n");
  } 
       else {
      Serial.println("Not azan\n");
}
   AzanCheckPlay (oghat, now);
      delay(100);
    if (BUSY_PIN == 1) {
        player.play(7);
    }
}
void printOghat(Oghat& oghat, Print& out) {
  Serial.print("Morning Azan  ");
  printTime(oghat.Morning, out);
  Serial.print("SunRise       ");
  printTime(oghat.SunRise, out);
  Serial.print("Noon Azan     ");
  printTime(oghat.Noon, out);
  Serial.print("Maghrib Azan  ");
  printTime(oghat.Maghrib, out);
}
