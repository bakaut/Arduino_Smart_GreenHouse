
/**************************************************************
 Author: Lebedev Nikolay
 Date created: 25-12-2017
 Date modufided: 10-01-2018

 Description:
 Read data from sd card (influx db line protocol) and send last 144 lines from text file to influx server via http post request.(gprs shild AI thinker A7)
 Used for monitoring greenhouse paramameters (temperature, pressure, etc.)
 For Arduino Mega SD card pinout: cs -- 53  mosi -- 51 sck -- 52 miso -- 50
 For Arduino Nano SD card pinout: cs -- 10  mosi -- 11 sck -- 13 miso -- 12
 Time to first request 50s\45s\40s\ debug on off, led on off
 **************************************************************/

#define TINY_GSM_MODEM_A7
//#define DEBUGMODE
//#define LEDDEBUG
#define MEGA
//#define NANO
#define SMS_TARGET "+79219614704"




#include <TinyGsmClient.h>

#include <BlockDriver.h>
#include <FreeStack.h>
#include <MinimumSerial.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <SysCall.h> 

// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "internet.mts.ru";
const char user[] = "mts";
const char pass[] = "mts";

const String HTTP_OK_STATUS="204"; //https://docs.influxdata.com/influxdb/v1.4/tools/api/#status-codes-and-responses-2


const uint8_t ERRORLED=3;
const uint8_t INFOLED=6;
const uint8_t SUCCESSLED=13;
#if defined MEGA
const uint8_t chipSelect = 53;
#endif
#if defined NANO
#include <SoftwareSerial.h>
SoftwareSerial SIM900A(3,2); // RX | TX
const uint8_t chipSelect = 10;
#endif
const uint8_t turn_on_gsm = 9;
const uint8_t gsm_power_btn = 7;
const uint8_t turn_1_arduino = 8;

File daily_file;

//Serial port for logging. Use Hardware Serial  on Mega, or software serial for Nano
#if defined DEBUGMODE
#define SerialDEBUG Serial
#endif

//Serial port for gsm module
#if defined MEGA
#define SerialAT Serial1
#endif
#if defined NANO
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(3,2); // RX | TX
#define SerialAT Serial//in my case i cant let software serial work for nano. And I use hardware serial
#endif



TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

const char server[] = "169.51.23.245";
const char resource[] = "/write?db=weather";
const int port = 30000;
String data_string,status_code,response;


void setup() {

  pinMode(turn_1_arduino, OUTPUT);
  pinMode(turn_on_gsm, OUTPUT);
  pinMode(gsm_power_btn, OUTPUT);
  pinMode(chipSelect, OUTPUT);
  pinMode(SUCCESSLED, OUTPUT);
  pinMode(INFOLED, OUTPUT);
  pinMode(ERRORLED, OUTPUT);

  #if defined DEBUGMODE
  SerialDEBUG.begin(115200);
  delay(1000);
  #endif


  // Set GSM module baud rate
  SerialAT.begin(115200);
  delay(1000);

  //turn on power gprs shild via mosfet
  #if defined DEBUGMODE
  SerialDEBUG.print("Turn on power gprs shild via mosfet...");
  #endif
  digitalWrite(turn_on_gsm, HIGH);
  #if defined DEBUGMODE
  SerialDEBUG.println("Done");
  #endif
 
  //power on gsm-gprs shild
  #if defined DEBUGMODE
  SerialDEBUG.print("Power on gsm-gprs shild...");
  #endif
  digitalWrite(gsm_power_btn, HIGH);
  delay(2500);
  digitalWrite(gsm_power_btn, LOW);
  delay(500);
  #if defined DEBUGMODE
  SerialDEBUG.println("Done");
  #endif

  //Restarting gsm modem
  startAndControl ("Restarting modem...", modem.restart(), 1);
  
  //String modemInfo = modem.getModemInfo();
  //SerialDEBUG.print("Modem: ");
  //SerialDEBUG.println(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");
}

void loop() {

  #if defined DEBUGMODE
  SerialDEBUG.println("Turn off 1-st arduino");
  #endif
  
  //turn off 1-st arduino
  digitalWrite(turn_1_arduino, LOW);
  
  //Initializing sd card
  SdFat SD;
  startAndControl ("Initializing sd...", SD.begin(chipSelect), 2);

  //Open file from sd card
  daily_file = SD.open("file2.txt", FILE_READ);
  startAndControl ("Open text file...", daily_file, 3);
  
  //Connect gprs to mts provider
  startAndControl ("Waiting for network...", modem.waitForNetwork(), 4);
  startAndControl ("Connecting to "+ String (apn)+ "...", modem.gprsConnect(apn, user, pass), 5);

  //Connect to web client ip port
  startAndControl ("Connecting to "+ String (server)+ "...", client.connect(server, port), 6);
  
  #if defined DEBUGMODE
  SerialDEBUG.print(F("Read txt file line by line and send data to web server: "));
  //Read only last 144 (24*6) lines (data for a last day)
  SerialDEBUG.println("File length: "+ String (daily_file.size()));
  SerialDEBUG.println("Set new file position 15676 simbols, approx 24*6=144 lines \r\n");
  #endif
  
  daily_file.seek(daily_file.size()-15676); 

  while (daily_file.available()) {

    data_string = daily_file.readStringUntil('\n');
    data_string = daily_file.readStringUntil('\n');
    data_string.trim();
    #if defined DEBUGMODE
    SerialDEBUG.println("Readed line: ");
    SerialDEBUG.println(data_string+"\r\n");        
    //modem.sendSMS(SMS_TARGET, data_string); //work mega
    #endif
    // Make a HTTP POST request:
    #if defined LEDDEBUG
    setled(ERRORLED,1,INFOLED,1,SUCCESSLED,1);
    #endif
    
    #if defined DEBUGMODE
    SerialDEBUG.println("Create and send post request: ");
    SerialDEBUG.println("POST " + String (resource) + " HTTP/1.1\r\nHost: "+ String (server) + "\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length:" + String (data_string.length()) +"\r\n\r\n"+ String (data_string));
    #endif
    client.print(String("POST ") + resource + " HTTP/1.1\r\n");
    client.print(String("Host: ") + server + "\r\n");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(data_string.length()); 
    client.println(); 
    client.print(data_string);
    client.println();
    
    //Read http request response
    unsigned long timeout = millis();
    int i=0;
    response = "";
    char a[500];
    while (client.connected() && millis() - timeout < 10000L) {
      // Create massive available data           
      while (client.available()) {
        char c = client.read();
        a[i]= c;
        response+=String(a[i]);
        i++;        
        timeout = millis();
       
    }

    }

    response.trim();
    status_code=String(a[9])+String(a[10])+String(a[11]);
    //modem.sendSMS(SMS_TARGET, status_code);
    
    if(status_code.equals(HTTP_OK_STATUS)) { 
      #if defined DEBUGMODE
      SerialDEBUG.println("Succesufuly write data to influx!"); 
      #endif
      //startAndControl ("Sending sms error request...", modem.sendSMS(SMS_TARGET, status_code+String(millis())), 1);
      #if defined LEDDEBUG
      successFlash(SUCCESSLED,7);
      #endif
      
    }
    else { 
      #if defined DEBUGMODE
      SerialDEBUG.println("Not Succesufuly write data to influx!");
      SerialDEBUG.println(response);
      #endif
      //In error send sms responce in 2 steps(sms length restriction)
      int l=i/2;
      String half1,half2;
      for (uint8_t k=0;k<l;k++){
        half1+=String(a[k]);
        }
      for (uint8_t k=l;k<i;k++){
        half2+=String(a[k]);
        }
      startAndControl ("Sending sms error request...", modem.sendSMS(SMS_TARGET, half1), 1);
      startAndControl ("Sending sms error request...", modem.sendSMS(SMS_TARGET, half2), 1);
      #if defined LEDDEBUG
      errorFlash(ERRORLED,7);
      #endif
    }
    #if defined LEDDEBUG
    setled(ERRORLED,0,INFOLED,0,SUCCESSLED,0);
    #endif
  }

  client.stop();
  #if defined DEBUGMODE
  SerialDEBUG.println("Server disconnected");
  #endif

  modem.gprsDisconnect();
  #if defined DEBUGMODE
  SerialDEBUG.println("GPRS disconnected");
  #endif

  //turn on 1-st arduino
  digitalWrite(turn_1_arduino, HIGH);

  //turn off power gprs shild via mosfet
  #if defined DEBUGMODE
  SerialDEBUG.println("turn on 1-st arduino");
  #endif
  digitalWrite(turn_on_gsm, LOW);
  #if defined DEBUGMODE
  SerialDEBUG.println("turn off power gprs shild via mosfet");
  #endif
  setled(ERRORLED,0,INFOLED,1,SUCCESSLED,1);
  
  //sleep for a 24 hour //turn off sd card pins
  #if defined DEBUGMODE
  SerialDEBUG.println("Wait for while");
  #endif
  while(1);
  
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


void startAndControl (String message,boolean command,uint8_t count) {
  #if defined DEBUGMODE
  SerialDEBUG.print(message);
  #endif
  boolean state = true;
  while (state) {
    #if defined LEDDEBUG
    infoFlash(INFOLED,1);
    #endif
    if (!command) {
        #if defined DEBUGMODE
        SerialDEBUG.print("Failed\r\n");
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
  
