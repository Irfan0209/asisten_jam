#include <Adafruit_SSD1306.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "TimeLib.h"


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define ADDRESS_OLED  0x3C
#define ADDRESS_LCD   0x27
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
LiquidCrystal_I2C lcd(ADDRESS_LCD,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display


#include <FluxGarage_RoboEyes.h>
roboEyes roboEyes; // create eyes

enum Mode{
  MODE_DEFAULT,
  MODE_SETTING,
  MODE_SETCLOCK,
  MODE_SETDATE
}; 
Mode mode = MODE_DEFAULT;

uint8_t jam;
uint8_t menit;
uint8_t detik;

uint8_t hari;
uint8_t tanggal;
uint8_t bulan;
uint16_t tahun;

uint8_t setJam;
uint8_t setMenit;
uint8_t setDetik;

uint16_t suhu = 100;

byte alarmOn[8] = {
  B00100,
  B01110,
  B01110,
  B01110,
  B11111,
  B00000,
  B00100,
  B00000,
};

byte alarmOff[8] = {
  B00100,
  B01010,
  B01011,
  B01010,
  B01110,
  B11111,
  B10000,
  B00100,
};

byte petik[8] = {
  B00000,
  B00100,
  B01010,
  B00100,
  B00000,
  B00000,
  B00000,
  B00000,
};

byte Suhu[8] = {
  B00100,
  B01010,
  B01010,
  B01010,
  B01110,
  B11111,
  B11111,
  B01110,
};

byte satu[8] = {
  B00100,
  B01100,
  B00100,
  B00100,
  B00100,
  B01110,
  B00000,
  B00000,
};

byte dua[8] = {
  B00100,
  B01010,
  B00010,
  B00100,
  B01000,
  B01110,
  B00000,
  B00000,
};

byte panah[8] = {
  B00000,
  B00100,
  B00010,
  B11111,
  B00010,
  B00100,
  B00000,
  B00000,
};

void setup() {
  Serial.begin(115200);
  lcd.init();   
  lcd.backlight();
  if(!display.begin(SSD1306_SWITCHCAPVCC, ADDRESS_OLED)) { // Address 0x3C or 0x3D
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  
  roboEyes.setPosition(DEFAULT); // eye position should be middle center
  roboEyes.setWidth(24, 24); // byte leftEye, byte rightEye
  roboEyes.setHeight(24, 24); // byte leftEye, byte rightEye
  roboEyes.setBorderradius(8, 8); // byte leftEye, byte rightEye
  roboEyes.setSpacebetween(10); // int space -> can also be negative
  roboEyes.close(); // start with closed eyes 

  setTime(23,43,00,24, 02, 25);
  lcd.createChar(0, alarmOn);
  lcd.createChar(1, alarmOff);
  lcd.createChar(2, petik);
  lcd.createChar(3, Suhu);
  lcd.createChar(4, satu);
  lcd.createChar(5, dua);
  lcd.createChar(6, panah);
}


void loop() {
  roboEyes.update(); // update eyes drawings
  roboEyes.setAutoblinker(ON, 3, 2); // Start auto blinker animation cycle -> bool active, int interval, int variation -> turn on/off, set interval between each blink in full seconds, set range for random interval variation in full seconds
  roboEyes.setIdleMode(ON, 2, 2); // Start idle animation cycle (eyes looking in random directions) -> turn on/off, set interval between each eye repositioning in full seconds, set range for random time interval variation in full seconds
  roboEyes.setCuriosity(ON);
  
  HOME();

}
//static int textPosition = 0;
void HOME(){
  const char *Hari[] = {"MINGGU","SENIN","SELASA","RABU","KAMIS","JUM'AT","SABTU"};
  const char text[] = "00:00";
  updateTime();
  static uint8_t counter;
//  static uint32_t   lsRn;
//  static bool state = false;
//  uint32_t          Tmr = millis(); 
  
  
  char buff_jam[13];
  char buff_date[20];

  snprintf(buff_jam,sizeof(buff_jam),(detik & 1)?"%02d:%02d %s":"%02d %02d %s",jam,menit, Hari[0]);
  snprintf(buff_date,sizeof(buff_date),"%02d-%02d-%04d" ,tanggal,bulan ,tahun);

 counter = textCount();
 switch(counter){
    case 0 :
     lcd.setCursor(0,1);
     lcd.print(buff_date);
    break;

    case 1 :
      lcd.setCursor(0,1);
      lcd.write(byte(0));
      lcd.write(byte(4));
      lcd.write(byte(6));
      lcd.print(text);
    break;

    case 2 :
      lcd.setCursor(0,1);
      lcd.write(byte(0));
      lcd.write(byte(5));
      lcd.write(byte(6));
      lcd.print(text);
    break;
  };
  //Serial.println("counter:" + String(counter));
  
  lcd.setCursor(0,0);
  lcd.print(buff_jam);
  //lcd.print("|");

//  lcd.setCursor(0,1);
//  lcd.print(buff_date);
  
  lcd.setCursor(11,1);
  lcd.write(byte(3));
  lcd.print(suhu);
  
  lcd.print("C");

  lcd.setCursor(13,0);
  lcd.write(byte(0));

  lcd.setCursor(15,0);
  lcd.write(byte(0));
  
}

// Fungsi untuk scrolling teks hanya di kolom 0-10
int textCount() {
  uint16_t currentMillis = millis();

  static uint8_t counter ;
  static uint16_t previousScrollMillis = 0;
  const uint32_t scrollInterval = 4000; // Waktu antar pergeseran (ms)
  static uint8_t last;
  
  
  if (currentMillis - previousScrollMillis >= scrollInterval) {
    previousScrollMillis = currentMillis;

    counter = (counter + 1) % 3;
    
  }
  if(counter != last){for(uint8_t i=0;i<11;i++){lcd.setCursor(i,1); lcd.print(" ");} last = counter;};
  return counter;
  
}


void SETTING(){
  
}

void SET_CLOCK(){
  
}

void SET_DATE(){
  
}

void updateTime(){
  jam   = hour();
  menit = minute();
  detik = second();

  tanggal = day();
  bulan   = month();
  tahun   = year();

  hari    = weekday();
}
