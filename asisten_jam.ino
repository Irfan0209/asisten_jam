#include <Adafruit_SSD1306.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "TimeLib.h"
#include "OneButton.h"

#define PIN_OK     2
#define PIN_UP     3
#define PIN_DOWN   4
#define PIN_SELECT 5

OneButton OK(PIN_OK, true);
OneButton UP(PIN_UP, true);
OneButton DOWN(PIN_DOWN, true);
OneButton SELECT(PIN_SELECT, true);

#define SLEEP_TIME 15000

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
  MODE_BLANK,
  MODE_DEFAULT,
  MODE_SETTING,
  MODE_SETCLOCK,
  MODE_ALARM1,
  MODE_TIMER
}; 
Mode mode ;

uint8_t jam;
uint8_t menit;
uint8_t detik;

uint8_t hari;
uint8_t tanggal;
uint8_t bulan;
uint16_t tahun;

uint8_t setTanggal;
uint8_t setBulan;
uint16_t setTahun=2000;

uint8_t setJam=1;
uint8_t setMenit=0;
uint8_t setDetik=0;

uint8_t jamAlarm;
uint8_t menitAlarm;
uint8_t detikAlarm;

uint16_t suhu = 100;

uint16_t TIMER = 2000;

char Alarm1[]={"12:00:00"};
char Clock[]={"11:00:00"};
char Date[]={"00-00-2000"};

char *textAlarm[]={"OFF","ON "};

bool sleepActive = false;
bool stateAlarm1 = false;
bool stateTimer = false;

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

uint8_t cursorSelect = 0;
uint8_t subLayer = 0;
uint8_t currentLayer = 0;

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

  OK.attachClick(click_ok);
  UP.attachClick(click_up);
  DOWN.attachClick(click_down);
  SELECT.attachClick(click_select);
}

uint8_t tampilan=1;
//uint8_t select;
bool flagSelect=false;

void loop() {
  roboEyes.update(); // update eyes drawings
  
  
  roboEyes.setAutoblinker(ON, 3, 2); // Start auto blinker animation cycle -> bool active, int interval, int variation -> turn on/off, set interval between each blink in full seconds, set range for random interval variation in full seconds
  roboEyes.setIdleMode(ON, 2, 2); // Start idle animation cycle (eyes looking in random directions) -> turn on/off, set interval between each eye repositioning in full seconds, set range for random time interval variation in full seconds
  roboEyes.setCuriosity(ON);
  //sleep();

  switch(mode){
    case MODE_DEFAULT :
      
      HOME();
    break;

    case MODE_SETTING :
      SETTING();
    break;

    case MODE_ALARM1 :
      ALARM1();
    break;

    case MODE_SETCLOCK :
      SET_CLOCK();
    break;

    case MODE_TIMER :
      SET_TIMER();
    break;
  };

  switch(tampilan){
    case 0 :
     // mode = MODE_BLANK;
    break;
    case 1 :
      mode = MODE_DEFAULT;
    break;

    case 2 :
      mode = MODE_SETTING;
    break;
  };

//  switch(subLayer){
//    case 0 :
//      //mode = MODE_BLANK;
//    break;
//    case 1 :
//      //lcd.clear();
//      mode = MODE_SETCLOCK;
//    break;
//    case 2 :
//      mode = MODE_ALARM1;
//    break;
//  };
  
  OK.tick();
  UP.tick();
  DOWN.tick();
  SELECT.tick();
}

void HOME(){
  const char *Hari[] = {"MINGGU","SENIN","SELASA","RABU","KAMIS","JUM'AT","SABTU"};
  const char *pasar[] = {"WAGE", "KLIWON", "LEGI", "PAHING", "PON"}; 
  const char text[] = "00:00";
  static uint8_t counter;
  static bool state;
  updateTime();
  
  state = stateDay();
  
  char buff_jam[13];
  char buff_date[20];

  snprintf(buff_jam,sizeof(buff_jam),(detik & 1)?"%02d:%02d %s":"%02d %02d %s",jam,menit, (state)?Hari[hari+1]:pasar[jumlahhari() % 5]);
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
  
  if(counter != last){
    for(uint8_t i=0;i<11;i++){
      lcd.setCursor(i,1); 
      lcd.print(" ");
     } 
    last = counter;
   };
  return counter;
}

int stateDay(){
  uint16_t tmr = millis();
  static uint16_t save = 0;
  static uint16_t interval = 3000;
  static bool state;
  static bool last;

  if((tmr - save) > interval){
    save = tmr;
    state = !state;
  }

  if(state != last){
    for(uint8_t i=6;i<12;i++){
      lcd.setCursor(i,0); 
      lcd.print(" ");
    } 
    last = state;
   }
  return state;
}


void SETTING(){
//     lcd.setCursor(15,cursorSelect ); 
//     lcd.write(byte(6));
//     lcd.setCursor(0,0);
//     lcd.print("ATUR JAM");
//     lcd.setCursor(0,1);
//     lcd.print("ATUR ALARM");
     switch(cursorSelect){
      case 0 :
        lcd.setCursor(15,0 ); 
        lcd.write(byte(6));
        lcd.setCursor(0,0);
        lcd.print("ATUR JAM");
        lcd.setCursor(0,1);
        lcd.print("ATUR ALARM");
     break;
     case 1 :
        lcd.setCursor(15,1 ); 
        lcd.write(byte(6));
        lcd.setCursor(0,0);
        lcd.print("ATUR JAM");
        lcd.setCursor(0,1);
        lcd.print("ATUR ALARM");
     break;
     case 2 :
        lcd.setCursor(15,0 ); 
        lcd.write(byte(6));
        lcd.setCursor(0,0);
        lcd.print("SET TIMER");
       
     break;
     };
 
}

void SET_CLOCK(){

char stateSave[]={"save"};
bool state = blinkText();

  lcd.setCursor(0,0);
  lcd.print("SET CLOCK: ");

if(cursorSelect==0 ){
  lcd.setCursor(11,0);
  if(blinkText()) lcd.print(setJam < 10 ? "0" + String(setJam) : String(setJam));
  else lcd.print("  ");
}else{
  lcd.setCursor(11,0);
  lcd.print(setJam < 10 ? "0" + String(setJam) : String(setJam));
}

lcd.setCursor(13,0);
lcd.print(":");
  
if(cursorSelect==1 ){
  lcd.setCursor(14,0);
  if(state) lcd.print(setMenit<10?"0" + String(setMenit):String(setMenit));
  else lcd.print("  ");
}else{
  lcd.setCursor(14,0);
  lcd.print(setMenit<10?"0" + String(setMenit):String(setMenit));
}

if(cursorSelect==2 ){
  lcd.setCursor(0,1);
  if(state) lcd.print(setTanggal<10?"0" + String(setTanggal):String(setTanggal));
  else lcd.print("  ");
}else{
  lcd.setCursor(0,1);
  lcd.print(setTanggal<10?"0" + String(setTanggal):String(setTanggal));
}

  lcd.setCursor(2,1);
  lcd.print("-");

if(cursorSelect==3 ){
  lcd.setCursor(3,1);
  if(state) lcd.print(setBulan<10?"0" + String(setBulan):String(setBulan));
  else lcd.print("  ");
}else{
  lcd.setCursor(3,1);
  lcd.print(setBulan<10?"0" + String(setBulan):String(setBulan));
}

  lcd.setCursor(5,1);
  lcd.print("-");

if(cursorSelect==4 ){
  lcd.setCursor(6,1);
  if(state) lcd.print(setTahun);
  else lcd.print("    ");
}else{
  lcd.setCursor(6,1);
  lcd.print(setTahun<10?"0" + String(setTahun):String(setTahun));
}

//if(cursorSelect==5 ){
//  lcd.setCursor(12,1);
//  if(state) lcd.print(stateSave);
//  else lcd.print("    ");
//}else{
//  lcd.setCursor(12,1);
//  lcd.print(stateSave);
//}

}

int blinkText(){
  static uint32_t save;
  static bool state = false;
  //uint32_t tmr = millis();

  if(millis() - save > 1000){
    save = millis();
    state = !state;
  }
  return state;
}

void ALARM1(){
  bool state = blinkText();
  
  Serial.println(F("set alarm active"));
  lcd.setCursor(0,0);
  lcd.print("ALARM 1: ");

  if(cursorSelect==0 ){
  lcd.setCursor(13,0);
  if(state) lcd.print(textAlarm[stateAlarm1]) ; 
  else lcd.print("    ");
  }else{
  lcd.setCursor(13,0);
  lcd.print(textAlarm[stateAlarm1]);
  }
  
  
  if(cursorSelect==1 ){
  lcd.setCursor(0,1);
  if(state) lcd.print(jamAlarm<10?"0" + String(jamAlarm):String(jamAlarm));
  else lcd.print("  ");
  }else{
  lcd.setCursor(0,1);
  lcd.print(jamAlarm<10?"0" + String(jamAlarm):String(jamAlarm));
  }

  lcd.setCursor(2,1);
  lcd.print(":");
  
  if(cursorSelect==2 ){
  lcd.setCursor(3,1);
  if(state) lcd.print(menitAlarm<10?"0" + String(menitAlarm):String(menitAlarm));
  else lcd.print("  ");
  }else{
  lcd.setCursor(3,1);
  lcd.print(menitAlarm<10?"0" + String(menitAlarm):String(menitAlarm));
  }

//  if(cursorSelect==3 ){
//  lcd.setCursor(12,1);
//  if(state) lcd.print("save");
//  else lcd.print("    ");
//  }else{
//  lcd.setCursor(12,1);
//  lcd.print("save");
//  }

//  lcd.setCursor(13,0);
//  lcd.print(textAlarm[0]);
//  lcd.setCursor(0,1);
//  lcd.print(Alarm1);
//  lcd.setCursor(12,1);
//  lcd.print("save");
}

void SET_TIMER(){
  bool state = blinkText();

  lcd.setCursor(0,0);
  lcd.print("SET TIMER: ");
  
  if(cursorSelect==0 ){
  lcd.setCursor(13,0);
  if(state) lcd.print(textAlarm[stateTimer]) ; 
  else lcd.print("    ");
  }else{
  lcd.setCursor(13,0);
  lcd.print(textAlarm[stateTimer]);
  }

  lcd.setCursor(0,1);
  lcd.print("timer:");
  if(cursorSelect==1 ){
  if(state) lcd.print(TIMER) ; 
  else lcd.print("    ");
  }else{
  //lcd.setCursor(0,1);
  lcd.print(TIMER);
  }
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

void sleep(){
 static uint32_t lastActivityTime;
 static bool isSleeping = false;

  // Cek apakah ada input tombol atau perubahan data
  if (sleepActive == true) { 
    //Serial.println(F("sleepActive active"));
    lastActivityTime = millis(); // Reset timer
    if (isSleeping) {
      lcd.backlight(); // Nyalakan kembali backlight, tampilan tetap ada
      isSleeping = false;
      //Serial.println(F("isSleeping active"));
    }
    sleepActive=false;
  }

  // Cek apakah sudah waktunya tidur
  if ((millis() - lastActivityTime) > SLEEP_TIME  && !isSleeping) {
    lastActivityTime=0;
    lcd.noBacklight(); // Hanya mematikan backlight, tidak menghapus tampilan
    isSleeping = true;
    //Serial.println(F("lcd sleep"));
  }
  
}

void click_ok(){
  
  if(mode == MODE_DEFAULT){tampilan=0; lcd.clear(); mode = MODE_SETTING;}//clearMenu();
  else if(mode == MODE_SETTING && cursorSelect == 0){lcd.clear(); cursorSelect=0; subLayer = 1; mode = MODE_SETCLOCK;}
  else if(mode == MODE_SETTING && cursorSelect == 1){lcd.clear(); cursorSelect=0; subLayer = 2; mode = MODE_ALARM1;}
  else if(mode == MODE_SETTING && cursorSelect == 2){lcd.clear(); cursorSelect=0; subLayer = 3; mode = MODE_TIMER;}
  else if(mode == MODE_SETCLOCK){ cursorSelect++; }//(cursorSelect+1) % 6; }
  else if(mode == MODE_ALARM1){ cursorSelect++; }
  //else if(mode == MODE_TIMER){ TIMER++; }
  Serial.println("mode:" + String(mode));
}

void click_up(){
//  sleepActive=true;
//  Serial.println(F("button active"));
  if(mode == MODE_SETTING){ 
    //clearSelect();
    if(cursorSelect>0){lcd.clear(); cursorSelect--;} 
    Serial.println("cursorSelect:" + String(cursorSelect));
  }
  //////mode set jam
  if(mode == MODE_SETCLOCK  && cursorSelect == 0){
    setJam = (setJam + 1) % 24; 
  }
  else if(mode == MODE_SETCLOCK  && cursorSelect == 1){
    setMenit = (setMenit + 1) % 60;
  }
  else if(mode == MODE_SETCLOCK  && cursorSelect == 2){
    (setTanggal > 31)? 1 : setTanggal++; 
  }
  else if(mode == MODE_SETCLOCK  && cursorSelect == 3){
    (setBulan > 12)? 1 : setBulan++;
  }
  else if(mode == MODE_SETCLOCK  && cursorSelect == 4){
    setTahun++; 
  }

  ///mode set alarm
  if(mode == MODE_ALARM1  && cursorSelect == 0){
    stateAlarm1 = true; 
  }
  else if(mode == MODE_ALARM1  && cursorSelect == 1){
    jamAlarm = (jamAlarm + 1) % 24; 
  }
  else if(mode == MODE_ALARM1  && cursorSelect == 2){
    menitAlarm = (menitAlarm + 1) % 60;
  }

  if(mode == MODE_TIMER){
    TIMER++;
  }

    Serial.println(F("up active"));
}

void click_down(){
  if(mode == MODE_SETTING ){ 
    //clearSelect ();
    if(cursorSelect < 2){lcd.clear(); cursorSelect++;} 
    
    Serial.println("cursorSelect:" + String(cursorSelect));
  }

  //mode set jam
  if(mode == MODE_SETCLOCK  && cursorSelect == 0){
    (setJam < 0)? 23 : setJam--;
  }
  else if(mode == MODE_SETCLOCK  && cursorSelect == 1){
    (setMenit < 0)? 23 : setMenit--; 
  }
  else if(mode == MODE_SETCLOCK  && cursorSelect == 2){
    (setTanggal < 1)? 31 : setTanggal--;
  }
  else if(mode == MODE_SETCLOCK  && cursorSelect == 3){
    (setBulan < 1)? 11 : setBulan--;
  }
  else if(mode == MODE_SETCLOCK  && cursorSelect == 4){
    (setTahun < 2000)? 2050 : setTahun--;
  }

  //mode set alarm
  if(mode == MODE_ALARM1  && cursorSelect == 0){
    stateAlarm1 = true; 
  }
  else if(mode == MODE_ALARM1  && cursorSelect == 1){
    (jamAlarm < 0)? 23 : jamAlarm--;
  }
  else if(mode == MODE_ALARM1  && cursorSelect == 2){
    (menitAlarm < 0)? 23 : menitAlarm--; 
  }

   if(mode == MODE_TIMER){
    TIMER--;
  }
    Serial.println(F("down active"));
}

void click_select(){
  if(mode == MODE_SETTING){ lcd.clear(); cursorSelect = 0; mode = MODE_DEFAULT; subLayer=0;}
  else if(mode == MODE_SETCLOCK ){lcd.clear(); cursorSelect = 0;  mode = MODE_SETTING; subLayer=0;}
  else if(mode == MODE_ALARM1 ){ lcd.clear(); cursorSelect = 1; mode = MODE_SETTING; subLayer=0;}
  else if(mode == MODE_TIMER ){ lcd.clear(); cursorSelect = 2; mode = MODE_SETTING; subLayer=0;}
  Serial.println(F("back active"));
}

void clearChar(int charPosition, int line){
  lcd.setCursor (charPosition,line);
  lcd.print(" ");
}
void clearSelect(){
  lcd.setCursor (15,0);
  lcd.print("  ");
  lcd.setCursor (15,1);
  lcd.print("  ");
}

void clearMenu(){
  for(int i=0;i<=16;i++){lcd.setCursor (i,0); lcd.print(" ");}
  for(int i=0;i<=16;i++){lcd.setCursor (i,1); lcd.print(" ");}
}

 
