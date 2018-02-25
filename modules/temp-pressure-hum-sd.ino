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

#define DEBUGMODE

//Serial port for logging. Use Hardware Serial  on Mega, or software serial for Nano
#if defined DEBUGMODE
#define SerialDEBUG Serial
#endif

/*Список, что переделат

Все переменные.
*/

const uint8_t SD_SC = 10; //SD card 
const uint8_t POWER_SWITCH = 5; //Turn on or turn off all sensor power via mosfet transistor
const uint8_t INFO_LED = 13;


File myFile,daily_file,one_file;

unsigned long pressure, aver_pressure, pressure_array[6], time_array[6];
unsigned long sumX, sumY, sumX2, sumXY;
int delta,current_day;
float a;
String check_data; //all sensor data in inflife line protocol


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
  
  #if defined DEBUGMODE
  SerialDEBUG.print("Turn on power control via mosfet...");
  #endif

  digitalWrite(POWER_SWITCH, HIGH);
  delay(100);

  #if defined DEBUGMODE
  SerialDEBUG.println("Done");
  #endif


  //Включаем датчик температуры и влажности
  
  //startAndControl ("Turn on temp and humanity sensor...", dht.begin(), 1);

  #if defined DEBUGMODE
  SerialDEBUG.print("Turn on temp and humanity sensor...");
  #endif
  dht.begin();
  #if defined DEBUGMODE
  SerialDEBUG.println("Done");
  #endif


  //Включаем датчик давления

  //startAndControl ("Turn on pressure sensor...", bmp.begin(), 1);
  #if defined DEBUGMODE
  SerialDEBUG.print("Turn on pressure sensor...");
  #endif
  bmp.begin();

  #if defined DEBUGMODE
  SerialDEBUG.println("Done");
  #endif
  
  pressure = aver_sens();          // найти текущее давление по среднему арифметическому
  for (byte i = 0; i < 6; i++) {   // счётчик от 0 до 5
    pressure_array[i] = pressure;  // забить весь массив текущим давлением
    time_array[i] = i;             // забить массив времени числами 0 - 5
  }
  
}

void loop() {

  #if defined DEBUGMODE
  SerialDEBUG.print("Turn on power control via mosfet...");
  #endif

  digitalWrite(POWER_SWITCH, HIGH);
  delay(500);

  #if defined DEBUGMODE
  SerialDEBUG.println("Done");
  #endif

  startAndControl ("Initializing sd...", SD.begin(SD_SC), 2);

 //   if (!SD.begin(SD_SC)) {
 //   blinking(INFO_LED,MINI,MINI,MAXI,MAXI,PAUSE);
 // return;
 // }

  bmp.readTemperature();  

  delay(200);
  
  
  if (RTC.read(tm)) {
    delay(700);
    }
  else {
    blinking(INFO_LED,MINI,MAXI,MAXI,MAXI,PAUSE);
    return;
    } 
  current_day = tm.Day;
   
  //Считываем влажность
  
  float h = dht.readHumidity();
  
  // Считываем температуру и давление
  
  float t = dht.readTemperature();
  float p = bmp.readPressure();
  
  
  // Проверка удачно прошло ли считывание.
  
  if (isnan(h) && isnan(t)&& isnan(p)) {
  
  blinking(INFO_LED,MAXI,MAXI,MAXI,MAXI,PAUSE);
  return;
  
  }

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
    // Вопрос: зачем столько раз пересчитывать всё отдельными формулами? Почему нельзя считать одной большой?
    // Ответ: а затем, что ардуинка не хочет считать такие большие числа сразу, и обязательно где-то наё*бывается,
    // выдавая огромное число, от которого всё идёт по пи*зде. Почему с матами? потому что устал отлаживать >:O
    delta = a * 6;                   // расчёт изменения давления

    myFile = SD.open("file.txt", FILE_WRITE);
    delay(20);

    check_data = "weather,location=uglovo,region=aerodrom temp="+String (t)+",hum="+String (h)+",pressure="+String (p)+",delta="+String (delta)+" "+String (makeTime(tm))+"000000000";
    
    // if the file opened okay, write to it:
    if (myFile) {
      myFile.println(check_data);
      // close the file:
      myFile.close();
      
      daily_file = SD.open(String (current_day), FILE_WRITE);
      delay(20);
      
      if (current_day) {
        daily_file.println(check_data);
        // close the file:
        daily_file.close();        
        }
      
      one_file = SD.open("current.txt", FILE_WRITE);
      delay(20);
      
      if (one_file) {
        one_file.println(check_data);
        // close the file:
        one_file.close();        
        }
        
    }
    
     else {
      // if the file didn't open, print an error:
      blinking(INFO_LED,MAXI,MAXI,MAXI,MAXI,5000);
      return;
    }
  if ( tm.Hour == 23 and tm.Minute > 49 ) {
  SD.remove("current.txt");        
  }
      //turn off power via mosfet
      delay(2000);
      digitalWrite(POWER_SWITCH, LOW);
      delay(600000);
   
  
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

void blinking (char led, unsigned short a, unsigned short b, unsigned short c, unsigned short d, unsigned short pause)
{ 
  pinMode(led, OUTPUT);
  flash (led,a,pause);
  flash (led,b,pause);
  flash (led,c,pause);
  flash (led,d,pause);
}

void flash (char led, unsigned short interval, unsigned short pause)
{
  digitalWrite(led, HIGH);
  delay(interval);
  digitalWrite(led, LOW);
  delay(pause);  
  }

void startAndControl (String message,boolean command,uint8_t count) {
  #if defined DEBUGMODE
  SerialDEBUG.print(message);
  #endif
  boolean state = true;
  while (state) {
    if (!command) {
        #if defined DEBUGMODE
        SerialDEBUG.println("Failed");
        #endif
        return;
       }
    else {
      state = false;
      #if defined DEBUGMODE
      SerialDEBUG.println("OK");
      #endif
      } 
   }
  } 