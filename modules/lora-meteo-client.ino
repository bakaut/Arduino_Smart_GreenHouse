#include "DHT.h"
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <SPI.h>
#include <LoRa.h>

#define DHTPIN A0 // номер пина, к которому подсоединен датчик температуры
#define DHTPINUPPER A1 // номер пина, к которому подсоединен датчик температуры выше
#define BMP280I2C 0x76
#define LORAFREQ 433E6
#define MINI 100
#define MAXI 700
#define PAUSE 500

#define DEBUGMODE
//#define LEDDEBUG


//Serial port for logging. Use Hardware Serial  on Mega, or software serial for Nano
#if defined DEBUGMODE
#define SerialDEBUG Serial
#endif

//Serial port for logging. Use Hardware Serial  on Mega, or software serial for Nano

#if defined DEBUGMODE
#define SerialDEBUG Serial
#endif

const uint8_t ERRORLED = 13;
float h,t,h_up,t_up,t_down,p,al,delta,a;

unsigned long pressure, aver_pressure, pressure_array[6], time_array[6];
unsigned long sumX, sumY, sumX2, sumXY;

String check_data,check_data_prefix; //all sensor data in inflife line protocol

Adafruit_BMP280 bmp; // I2C
DHT dht(DHTPIN, DHT22);//Датчик температуры и влажности наружний
DHT dhtUpper(DHTPINUPPER, DHT11);
  
void setup() {

  #if defined DEBUGMODE
  SerialDEBUG.begin(9600);
  delay(100);
  #endif

  /* Default settings from bpm280 datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_NONE,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_NONE,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_OFF      /* Filtering. */
                  );

  //Включаем датчик температуры и влажности
  dht.begin();
  dhtUpper.begin();
  
  //Включаем датчик давления
  startAndControl ("Starting bpm 280...", bmp.begin(BMP280I2C),1);
  //bmp.begin(BMP280I2C); 
  
  //Включаем передатчик
  startAndControl ("Starting lora radio...", LoRa.begin(LORAFREQ),2);
  
  pressure = aver_sens();          // найти текущее давление по среднему арифметическому
  for (byte i = 0; i < 6; i++) {   // счётчик от 0 до 5
    pressure_array[i] = pressure;  // забить весь массив текущим давлением
    time_array[i] = i;             // забить массив времени числами 0 - 5
  }

}

void loop() {

  h = dht.readHumidity();
  t = dht.readTemperature();

  h_up = dhtUpper.readHumidity();
  t_up = dhtUpper.readTemperature();

  p = bmp.readPressure();
  t_down = bmp.readTemperature();
  al = bmp.readAltitude(70); //Высоты над уровнем моря в метрах в Углово

 
  startAndControl ("Check readed data from sensors temp...", isdigit(t) , 3);
  startAndControl ("Check readed data from sensors... temp2", isdigit(t_up) , 3);
  startAndControl ("Check readed data from sensors... hum", isdigit(h) , 3);
  startAndControl ("Check readed data from sensors... hum2", isdigit(h_up) , 3);
  startAndControl ("Check readed data from sensors... pressure", isdigit(p) , 3);

  // put your main code here, to run repeatedly:

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


  check_data_prefix = "weather,location=uglovo,region=aerodrom ";
  check_data = "te="+String (t)+",ti="+String (t_up)+",td="+String (t-t_up)+",h="+String (h)+",hi="+String (h_up)+",hd="+String (h_up-h)+",p="+String (p)+",pd="+String (delta)+"\r\n";
  
  #if defined DEBUGMODE
  SerialDEBUG.print(check_data_prefix+check_data);
  #endif

  LoRa.beginPacket();
  LoRa.print(check_data_prefix+check_data);
  LoRa.endPacket();
  delay(5000);

}

void startAndControl (String message,boolean command,uint8_t count) {
  #if defined DEBUGMODE
  SerialDEBUG.print(message+"\r\n");
  #endif
  boolean state = true;
  while (state) {
    if (!command) {
        #if defined DEBUGMODE
        SerialDEBUG.println(" Failed");
        #endif
        #if defined LEDDEBUG
          errorFlash(ERRORLED,count);        
        #endif
        return;
       }
    else {
      state = false;
      #if defined DEBUGMODE
      SerialDEBUG.println(" OK");
      #endif
      } 
   }
  }

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

void errorFlash (uint8_t errorled, uint8_t count)

{   
    flash(errorled,count,100,100);
    digitalWrite(errorled, HIGH);
    delay(2000);
    digitalWrite(errorled, LOW);
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
