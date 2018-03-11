#include <Time.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <Wire.h>
#include "DHT.h"
#include <SPI.h>
#include <Adafruit_BMP085.h>

#include <BlockDriver.h>
#include <FreeStack.h>
#include <MinimumSerial.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <SysCall.h> 

#define DHTPIN A0 // номер пина, к которому подсоединен датчик температуры
#define MINI 100
#define MAXI 700
#define PAUSE 500


//#define DEBUGMODE
#define LEDDEBUG

//Serial port for logging. Use Hardware Serial  on Mega, or software serial for Nano
#if defined DEBUGMODE
#define SerialDEBUG Serial
#endif


const uint8_t SD_SC = 10; //SD card 
const uint8_t POWER_SWITCH = 5; //Turn on or turn off all sensor power via mosfet transistor


#if defined LEDDEBUG
const uint8_t ERRORLED=2;
const uint8_t INFOLED=3;
const uint8_t SUCCESSLED=4;
#endif


unsigned long pressure, aver_pressure, pressure_array[6], time_array[6];
unsigned long sumX, sumY, sumX2, sumXY;
int delta,current_day;
float a,h,t,p;
String check_data; //all sensor data in inflife line protocol
String all_data = "file.txt", current_data = "current.txt", current_day_file;
File all_data_file, current_data_file, current_day_file_file;


//Датчик температуры и влажности
DHT dht(DHTPIN, DHT22);
//Датчик давления
Adafruit_BMP085 bmp;
//SD Карта
SdFat SD;
//Часы. Tyny RTC clock
tmElements_t tm;

void setup() {

  #if defined DEBUGMODE
  SerialDEBUG.begin(9600);
  delay(100);
  #endif

  //Включаем питание датчиков через транзистор
  pinMode(POWER_SWITCH, OUTPUT);

  digitalWrite(POWER_SWITCH, HIGH);
  delay(500);


  //Включаем датчик температуры и влажности
  dht.begin();

  //Включаем датчик давления
  bmp.begin();
  
  
  #if defined LEDDEBUG
    pinMode(ERRORLED, OUTPUT);
    pinMode(INFOLED, OUTPUT);
    pinMode(SUCCESSLED, OUTPUT);
    setled(ERRORLED,1,INFOLED,1,SUCCESSLED,1);
    delay(1000);
    setled(ERRORLED,0,INFOLED,0,SUCCESSLED,1);
    delay(1000);
    setled(ERRORLED,1,INFOLED,0,SUCCESSLED,0);
    delay(1000);
    setled(ERRORLED,0,INFOLED,1,SUCCESSLED,0);
    delay(1000);
    setled(ERRORLED,0,INFOLED,0,SUCCESSLED,0);
  #endif

  pressure = aver_sens();          // найти текущее давление по среднему арифметическому
  for (byte i = 0; i < 6; i++) {   // счётчик от 0 до 5
    pressure_array[i] = pressure;  // забить весь массив текущим давлением
    time_array[i] = i;             // забить массив времени числами 0 - 5
  }
  
}

void loop() {
  
  digitalWrite(POWER_SWITCH, HIGH);
  delay(1000);

  startAndControl ("Initializing sd...", SD.begin(SD_SC), 2);
  
  //Read temperature

  startAndControl ("Read temperature from pressure sensor ...", bmp.readTemperature(), 3);
  delay(100);
  
  //Read date time
  startAndControl ("Read date time...", RTC.read(tm), 4);
  delay(700);
  current_day = tm.Day;
  current_day_file = String (current_day);
   
  //Считываем влажность, температуру. давление
  h = dht.readHumidity();
  t = dht.readTemperature();
  p = bmp.readPressure();
  
   
  // Проверка удачно прошло ли считывание.
  
  startAndControl ("Check readed data from sensors...", isnan(h) && isnan(t)&& isnan(p), 5);

  pressure = aver_sens();                          // найти текущее давление по среднему арифметическому
  for (byte i = 0; i < 5; i++) {                   // счётчик от 0 до 5 (да, до 5. Так как 4 меньше 5)
    pressure_array[i] = pressure_array[i + 1];     // сдвинуть массив давлений КРОМЕ ПОСЛЕДНЕЙ ЯЧЕЙКИ на шаг назад
  }
  pressure_array[5] = pressure;                    // последний элемент массива теперь - новое давление

  sumX = 0;
  sumY = 0;
  sumX2 = 0;
  sumXY = 0;
  for (int i = 0; i < 6; i++) {                    // для всех элементов массива
    sumX += time_array[i];
    sumY += (long)pressure_array[i];
    sumX2 += time_array[i] * time_array[i];
    sumXY += (long)time_array[i] * pressure_array[i];
  }

  a = 0;
  a = (long)6 * sumXY;             // расчёт коэффициента наклона приямой
  a = a - (long)sumX * sumY;
  a = (float)a / (6 * sumX2 - sumX * sumX);
  delta = a * 6;                   // расчёт изменения давления
  
  check_data = "weather,location=uglovo,region=aerodrom temp="+String (t)+",hum="+String (h)+",pressure="+String (p)+",delta="+String (delta)+" "+String (makeTime(tm))+"000000000";
  
  write_to_sd (all_data, all_data_file, check_data, 6);

  write_to_sd (current_day_file, current_data_file, check_data, 7);
  
  write_to_sd (current_data, current_day_file_file, check_data, 8);

  if ( tm.Hour == 23 and tm.Minute >45 ) {
    startAndControl ("Remove current day file... ", SD.remove("current.txt"), 9);
     
  }
  
  //turn off power via mosfet
  delay(2000);
  digitalWrite(POWER_SWITCH,LOW);
  delay(2000);
   
  
  }

// среднее арифметичсекое от давления
long aver_sens() {
  pressure = 0;
  for (byte i = 0; i < 10; i++) {
    pressure += bmp.readPressure();
  }
  aver_pressure = pressure / 10;
  return aver_pressure;
}


#if defined LEDDEBUG
void flash (uint8_t led, uint8_t count, unsigned short interval, unsigned short pause)
{ 
  uint8_t  i;
  
  for (i=0; i < count; i++) {
      digitalWrite(led, HIGH);
      delay(interval);
      digitalWrite(led, LOW);
      delay(pause);     
    }
  
 
  }

void setled (uint8_t led1,uint8_t state1,uint8_t led2,uint8_t state2,uint8_t led3,uint8_t state3)
{ 

  digitalWrite(led1, state1);
  digitalWrite(led2, state2);
  digitalWrite(led3, state3);
    
}

void errorFlash (uint8_t errorled, uint8_t count)
{   
    flash(errorled,count,100,100);
    digitalWrite(errorled, HIGH);
    delay(2000);
    digitalWrite(errorled, LOW);
  }

void successFlash (uint8_t successled, uint8_t count)
{
    flash(successled,count,100,100);
}

void infoFlash (uint8_t infoled, uint8_t count)
{
    flash(infoled,count,70,70);
    delay(100);
}
#endif


void startAndControl (String message,boolean command,uint8_t count) {
  #if defined DEBUGMODE
  SerialDEBUG.print(message+"\r\n");
  #endif
  boolean state = true;
  while (state) {
    if (!command) {
        #if defined DEBUGMODE
        SerialDEBUG.print("Failed");
        #endif
        #if defined LEDDEBUG
        errorFlash(ERRORLED,count);
        #endif
        return;
       }
    else {
      state = false;
      #if defined LEDDEBUG
      successFlash(SUCCESSLED,count);
      #endif
      #if defined DEBUGMODE
      SerialDEBUG.println("OK");
      #endif
      } 
   }
  }


void write_to_sd (String filename, File file_var, String check_data, uint8_t count) {

  startAndControl ("Open file..." + filename, file_var = SD.open(filename, FILE_WRITE), count);
  delay(20);

  if(file_var) {
    file_var.println(check_data);
    file_var.close();        
  }

  else {
    file_var.close();
    #if defined LEDDEBUG
      errorFlash(ERRORLED,count);        
    #endif
    #if defined DEBUGMODE
      SerialDEBUG.println("Cannot write to file "+String(filename));
    #endif
    #if defined LEDDEBUG
      successFlash(SUCCESSLED,count);        
    #endif
    return; 
  }

}
