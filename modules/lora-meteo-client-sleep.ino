/**************************************************************************/
#include <avr/sleep.h>      // AVR MCU power management
#include <avr/power.h>     // disable/anable AVR MCU peripheries (Analog Comparator, ADC, USI, Timers/Counters)
#include <avr/wdt.h>         // AVR MCU watchdog timer
#include <avr/io.h>           // includes the apropriate AVR MCU IO definitions
#include <avr/interrupt.h> // manipulation of the interrupt flags

/**************************************************************************/
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

const uint8_t ERRORLED = 13;
float h,t,h_up,t_up,t_down,p,al,delta,a;

unsigned long pressure, aver_pressure, pressure_array[6], time_array[6];
unsigned long sumX, sumY, sumX2, sumXY;

String check_data,check_data_prefix; //all sensor data in inflife line protocol

Adafruit_BMP280 bmp; // I2C
DHT dht(DHTPIN, DHT22);//Датчик температуры и влажности наружний
DHT dhtUpper(DHTPINUPPER, DHT11);

// This variable is made volatile because it is changed inside
// an interrupt function
volatile int f_wdt=3; // 3*8 = 24 seconds

void setup() {
  setupWatchDogTimer();
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
  startAndControl ("Sent lora packet...", LoRa.endPacket() , 6);
}

void loop() {
  // Wait until the watchdog have triggered a wake up.

  if(f_wdt != 1) {
    return;
  }

  //here make someone

  // clear the flag so we can run above code again after the MCU wake up
  f_wdt = 0;

  // Re-enter sleep mode.
  enterSleep();
  startAndControl ("Enter sleep ...", bmp.readTemperature(), 7);

}


void set_all_pins( bool mode, uint8_t pinmode ) {
  for (int i = 0; i < A1; i++) {
      pinMode(i, pinmode); //OUTPUT INPUT
      digitalWrite(i, mode); //LOW HIGH
    }
}

// Watchdog Interrupt Service. This is executed when watchdog timed out.
ISR(WDT_vect) {
  if(f_wdt == 0) {
    // here we can implement a counter the can set the f_wdt to true if
    // the watchdog cycle needs to run longer than the maximum of eight
    // seconds.
    f_wdt=1;
  }
  f_wdt--;
}

// Enters the arduino into sleep mode.
void enterSleep(void)
{
  // There are five different sleep modes in order of power saving:
  // SLEEP_MODE_IDLE - the lowest power saving mode
  // SLEEP_MODE_ADC
  // SLEEP_MODE_PWR_SAVE
  // SLEEP_MODE_STANDBY
  // SLEEP_MODE_PWR_DOWN - the highest power saving mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();

  // Now enter sleep mode.
  sleep_mode();

  // The program will continue from here after the WDT timeout

  // First thing to do is disable sleep.
  sleep_disable();

  // Re-enable the peripherals.
  power_all_enable();
}

// Setup the Watch Dog Timer (WDT)
void setupWatchDogTimer() {
  // The MCU Status Register (MCUSR) is used to tell the cause of the last
  // reset, such as brown-out reset, watchdog reset, etc.
  // NOTE: for security reasons, there is a timed sequence for clearing the
  // WDE and changing the time-out configuration. If you don't use this
  // sequence properly, you'll get unexpected results.

  // Clear the reset flag on the MCUSR, the WDRF bit (bit 3).
  MCUSR &= ~(1<<WDRF);

  // Configure the Watchdog timer Control Register (WDTCSR)
  // The WDTCSR is used for configuring the time-out, mode of operation, etc

  // In order to change WDE or the pre-scaler, we need to set WDCE (This will
  // allow updates for 4 clock cycles).

  // Set the WDCE bit (bit 4) and the WDE bit (bit 3) of the WDTCSR. The WDCE
  // bit must be set in order to change WDE or the watchdog pre-scalers.
  // Setting the WDCE bit will allow updates to the pre-scalers and WDE for 4
  // clock cycles then it will be reset by hardware.
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  /**
   *  Setting the watchdog pre-scaler value with VCC = 5.0V and 16mHZ
   *  WDP3 WDP2 WDP1 WDP0 | Number of WDT | Typical Time-out at Oscillator Cycles
   *  0    0    0    0    |   2K cycles   | 16 ms
   *  0    0    0    1    |   4K cycles   | 32 ms
   *  0    0    1    0    |   8K cycles   | 64 ms
   *  0    0    1    1    |  16K cycles   | 0.125 s
   *  0    1    0    0    |  32K cycles   | 0.25 s
   *  0    1    0    1    |  64K cycles   | 0.5 s
   *  0    1    1    0    |  128K cycles  | 1.0 s
   *  0    1    1    1    |  256K cycles  | 2.0 s
   *  1    0    0    0    |  512K cycles  | 4.0 s
   *  1    0    0    1    | 1024K cycles  | 8.0 s
  */
  WDTCSR  = (1<<WDP3) | (0<<WDP2) | (1<<WDP0) | (1<<WDP1);
  // Enable the WD interrupt (note: no reset).
  WDTCSR |= _BV(WDIE);
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
