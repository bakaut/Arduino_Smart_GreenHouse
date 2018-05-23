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
#define DHTPIN2 A1 // номер пина, к которому подсоединен датчик температуры внутренний
#define MINI 100
#define MAXI 700
#define PAUSE 500


const uint8_t SD_SC = 10; //SD card 
const uint8_t POWER_SWITCH = 5; //Turn on or turn off all sensor power via mosfet transistor
const uint8_t ERRORLED = 13;


unsigned long pressure, aver_pressure, pressure_array[6], time_array[6];
unsigned long sumX, sumY, sumX2, sumXY;
int delta,current_day;
float a,h,t,p,t_int,hi;
String check_data_prefix; //all sensor data in inflife line protocol
String all_data = "file.txt", current_data = "current.txt", current_day_file;
File all_data_file, current_data_file, current_day_file_file;


//Датчик температуры и влажности
DHT dht(DHTPIN, DHT22);
DHT dhti(DHTPIN2, DHT11);
//Датчик давления
Adafruit_BMP085 bmp;
//SD Карта
SdFat SD;
//Часы. Tyny RTC clock
tmElements_t tm;

void setup() {

 //Включаем питание датчиков через транзистор
  pinMode(POWER_SWITCH, OUTPUT);

  digitalWrite(POWER_SWITCH, HIGH);
  delay(100);


  //Включаем датчик температуры и влажности
  dht.begin();
  dhti.begin();

  //Включаем датчик давления
  bmp.begin();


  pressure = aver_sens();          // найти текущее давление по среднему арифметическому
  for (byte i = 0; i < 6; i++) {   // счётчик от 0 до 5
    pressure_array[i] = pressure;  // забить весь массив текущим давлением
    time_array[i] = i;             // забить массив времени числами 0 - 5
  }
  
}

void loop() {

  digitalWrite(POWER_SWITCH, HIGH);
  delay(500);
  SD.begin(SD_SC);

  //Read temperature
  bmp.readTemperature();
  delay(100);
  
  //Read date time
  RTC.read(tm);
  delay(700);
  current_day = tm.Day;
  current_day_file = String (current_day);
   
  //Считываем влажность, температуру. давление
  h = dht.readHumidity();
  hi = dhti.readHumidity();
  t = dht.readTemperature();
  p = bmp.readPressure();
  t_int = bmp.readTemperature();
  
   
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
  
  //140 symvol restriction. Make 2 String variable.
  check_data_prefix = "weather,location=uglovo,region=aerodrom ";
  const String check_data = "te="+String (t)+",ti="+String (t_int)+",td="+String (t-t_int)+",h="+String (h)+",hi="+String (hi)+",p="+String (p)+",pd="+String (delta)+" "+String (makeTime(tm));
  
  
  write_to_sd (all_data, all_data_file, check_data, check_data_prefix);
  write_to_sd (current_day_file, current_data_file, check_data, check_data_prefix);
  write_to_sd (current_data, current_day_file_file, check_data, check_data_prefix);

  if ( tm.Hour == 23 and tm.Minute >45 ) {
     SD.remove("current.txt");  
  }

  //turn off power via mosfet
  delay(2000);
  digitalWrite(POWER_SWITCH, LOW);
  delay(600000);//10 minutes

  
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



void write_to_sd (String filename, File file_var, String check_data, String check_data_prefix) {
  file_var = SD.open(filename, FILE_WRITE);
  delay(100);
  file_var.print(check_data_prefix);
  file_var.position();
  file_var.print(check_data);
  file_var.position();
  file_var.println("000000000");
  file_var.close();
  delay(100);      
}
