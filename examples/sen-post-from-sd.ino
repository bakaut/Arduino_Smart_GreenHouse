
/**************************************************************
 Author: Lebedev Nikolay
 Date created: 25-12-2017
 Date modufided: 25-05-2018

 Description:
 Read data from sd card (influx db line protocol) and send last 144 lines from text file to influx server via http post request.(gprs shild AI thinker A7)
 Used for monitoring greenhouse paramameters (temperature, pressure, etc.)
 
 For Arduino Nano SD card pinout: cs -- 10  mosi -- 11 sck -- 13 miso -- 12
 Time to first request 50s\45s\40s\ debug on off, led on off

 SIMPLE VARIANT
 **************************************************************/

#define TINY_GSM_MODEM_A7
#define SMS_TARGET "+79219614704"
#define SerialAT Serial

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

const uint8_t chipSelect = 10;
const uint8_t turn_on_gsm = 9;
const uint8_t gsm_power_btn = 7;
const uint8_t turn_1_arduino = 8;

File daily_file;

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

  // Set GSM module baud rate
  SerialAT.begin(115200);
  delay(1000);

  //turn on power gprs shild via mosfet
  digitalWrite(turn_on_gsm, HIGH);
  //power on gsm-gprs shild
  digitalWrite(gsm_power_btn, HIGH);
  delay(2500);
  digitalWrite(gsm_power_btn, LOW);
  delay(500);

  //Restarting gsm modem
  modem.restart();
  
}

void loop() {
 
  //turn off 1-st arduino
  digitalWrite(turn_1_arduino, LOW);
  
  //Initializing sd card
  SdFat SD;
  SD.begin(chipSelect);

  //Open file from sd card
  daily_file = SD.open("file2.txt", FILE_READ);
  
  //Connect gprs to mts provider
  modem.waitForNetwork();
  modem.gprsConnect(apn, user, pass);

  //Connect to web client ip port
  client.connect(server, port);
  

  //"Read txt file line by line and send data to web server: "));
  //Read only last 144 (24*6) lines (data for a last day)
  //SerialDEBUG.println("File length: "+ String (daily_file.size()));
  //SerialDEBUG.println("Set new file position 15676 simbols, approx one day 24*6=144 lines \r\n");
  
  daily_file.seek(daily_file.size()-15676); 

  while (daily_file.available()) {
    data_string = daily_file.readStringUntil('\n');
    data_string.trim();      
    modem.sendSMS(SMS_TARGET, data_string); //work mega

    // Make a HTTP POST request:

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
    modem.sendSMS(SMS_TARGET, status_code+" "+String(millis()));
    
  client.stop();

  modem.gprsDisconnect();

  //turn on 1-st arduino
  digitalWrite(turn_1_arduino, HIGH);

  //turn off power gprs shild via mosfet
  digitalWrite(turn_on_gsm, LOW);
  
  //sleep for a 24 hour //turn off sd card pins
  //while(1);
  
}
}
