//#include <Time.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
//#include <Wire.h>
#include "DHT.h"
#include <SD.h>
#include <SPI.h>
#include <Adafruit_BMP085.h>


#define DHTPIN A0 // номер пина, к которому подсоединен датчик

const int chipSelect = 10;
File myFile;

unsigned long pressure, aver_pressure, pressure_array[6], time_array[6];
unsigned long sumX, sumY, sumX2, sumXY;
int delta;
float a;

// Раскомментируйте в соответствии с используемым датчиком

// Инициируем датчик
DHT dht(DHTPIN, DHT22);

//Для измерения давления
Adafruit_BMP085 bmp;


// среднее арифметичсекое от давления
long aver_sens() {
  pressure = 0;
  for (byte i = 0; i < 10; i++) {
    pressure += bmp.readPressure();
  }
  aver_pressure = pressure / 10;
  return aver_pressure;
}

void setup() {
  Serial.begin(9600);
  
  dht.begin();
  if (!SD.begin(chipSelect)) {
  Serial.println("Card failed, or not present");
  // don't do anything more:
  return;
  }
  
  if (!bmp.begin()) {
  Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  while (1) {}
  }
  
  pressure = aver_sens();          // найти текущее давление по среднему арифметическому
  for (byte i = 0; i < 6; i++) {   // счётчик от 0 до 5
    pressure_array[i] = pressure;  // забить весь массив текущим давлением
    time_array[i] = i;             // забить массив времени числами 0 - 5
  }
  
}

void loop() {
  bmp.readTemperature();  
  //bmp.readAltitude();
  //bmp.readAltitude(100550);//109194 for 731 meter
  
  delay(500);
  
  tmElements_t tm;
  myFile = SD.open("file.txt", FILE_WRITE);
  
  if (RTC.read(tm)) {
    delay(2000);
    }
  //else {} //set time
  //Считываем влажность
  
  float h = dht.readHumidity();
  
  // Считываем температуру
  
  float t = dht.readTemperature();
  float p = bmp.readPressure();
  
  
  // Проверка удачно прошло ли считывание.
  
  if (isnan(h) || isnan(t)) {
  
  Serial.println("Не удается считать показания");
  
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
    delta = a * 6;                   // расчёт изменения давления
    
    // if the file opened okay, write to it:
    if (myFile) {
      myFile.print("weather,location=uglovo,region=aerodrom temp=");
      myFile.print(t);
      myFile.print(",hum=");
      myFile.print(h);
      myFile.print(",pressure=");
      myFile.print(p);
      myFile.print(",delta=");
      myFile.print(delta);
      myFile.print(" ");
      myFile.print(makeTime(tm));
      myFile.println("000000000");
      // close the file:
      myFile.close();
      
      
      delay(600000);

    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }
  
  }
