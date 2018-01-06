
/**************************************************************
 Author: Lebedev Nikolay
 Date created: 25-12-2017
 Date modufided: 05-01-2018

 Description:
 Read data from sd card (influx db line protocol) and send last 144 lines from text file to influx server via http post request.(gprs shild AI thinker A7)
 Used for monitoring greenhouse paramameters (temperature, pressure, etc.)
 **************************************************************/

#define TINY_GSM_MODEM_A7

#include <TinyGsmClient.h>

#include <BlockDriver.h>
#include <FreeStack.h>
#include <MinimumSerial.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <SysCall.h> 

#define ERRORLED 3
#define SUCCESSLED 5
#define INFOLED 6


// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "internet.mts.ru";
const char user[] = "mts";
const char pass[] = "mts";
const String HTTP_OK_STATUS="204"; //https://docs.influxdata.com/influxdb/v1.4/tools/api/#status-codes-and-responses-2

File daily_file;

// Use Hardware Serial on Mega, Leonardo, Micro

//#define SerialDEBUG Serial
#define SerialAT Serial

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

const char server[] = "169.51.23.245";
const char resource[] = "/write?db=weather";
const int port = 30000;
const int chipSelect = 10;
String data_string,status_code;


void setup() {
  pinMode(4, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(10, OUTPUT);
  
  starting(ERRORLED,SUCCESSLED,INFOLED);
  stoping(ERRORLED,SUCCESSLED,INFOLED);
  waiting(SUCCESSLED,ERRORLED,10);
  stoping(ERRORLED,SUCCESSLED,INFOLED);
  
  // Set console baud rate
  //SerialDEBUG.begin(115200);
  //delay(1000);


  // Set GSM module baud rate
  SerialAT.begin(115200);
  delay(1000);

  //turn on power gprs shild via mosfet

  //SerialDEBUG.print("Turn on power gprs shild via mosfet...");
  digitalWrite(9, HIGH);
  //SerialDEBUG.println("Done");
  flash(SUCCESSLED,1,100,100);

  //power on gsm-gprs shild
  //SerialDEBUG.print("Power on gsm-gprs shild...");
  
  digitalWrite(7, HIGH);
  delay(2500);
  digitalWrite(7, LOW);
  delay(500);
  //SerialDEBUG.println("Done");
  flash(SUCCESSLED,1,100,100);
  
  waiting(SUCCESSLED,ERRORLED,10);

  //SerialDEBUG.print("Restarting modem...");
  
  if (!modem.restart()) {
      //SerialDEBUG.print("Failed");
      flash(ERRORLED,1,100,100);
      analogWrite(ERRORLED, 255);
      delay(2000);
      return;
    }


  //SerialDEBUG.println("OK");
  flash(SUCCESSLED,1,100,100);

//  String modemInfo = modem.getModemInfo();
  //Serial.print("Modem: ");
  //Serial.println(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");
}

void loop() {
  //SerialDEBUG.listen();
  //SerialDEBUG.println("Turn off 1-st arduino");

  waiting(SUCCESSLED,ERRORLED,10);
  
  //turn off 1-st arduino
  
  digitalWrite(8, LOW);

  //SerialDEBUG.print(F("Initializing sd..."));
  SdFat SD;
 
  //init sd card
  if (!SD.begin(chipSelect)) {
  // don't do anything more:
  //SerialDEBUG.println(F("Failed"));
  flash(ERRORLED,1,100,100);
  analogWrite(ERRORLED, 255);
  delay(2000);
  return;
  }
  //SerialDEBUG.println("Done");
  
  flash(SUCCESSLED,2,100,100);
  
  //SerialDEBUG.print("Open text file...");
  
  daily_file = SD.open("file2.txt", FILE_READ);
  
  if (!daily_file) {
    //SerialDEBUG.print("The text file.txt file cannot be opened");
    flash(ERRORLED,2,100,100);
    analogWrite(ERRORLED, 255);
    delay(2000);
    return;
  }
  //SerialDEBUG.println("Done");
  flash(SUCCESSLED,2,100,100);


  //Connect gprs
  //SerialDEBUG.print(F("Waiting for network..."));
  waiting(SUCCESSLED,ERRORLED,10);
  if (!modem.waitForNetwork()) {
    //SerialDEBUG.println("Failed");
      flash(ERRORLED,3,100,100);
      analogWrite(ERRORLED, 255);
      delay(2000);
      return;
  }
   stoping(ERRORLED,SUCCESSLED,INFOLED);
   //SerialDEBUG.println(" OK");
   flash(SUCCESSLED,3,100,100);

  //SerialDEBUG.print(F("Connecting to "));
  //SerialDEBUG.print(apn);
  //SerialDEBUG.print("...");
  waiting(SUCCESSLED,ERRORLED,10);
  if (!modem.gprsConnect(apn, user, pass)) {
     //SerialDEBUG.println(" fail");
      flash(ERRORLED,4,100,100);
      analogWrite(ERRORLED, 255);
      delay(2000);
      return;
  }
  stoping(ERRORLED,SUCCESSLED,INFOLED);
  //SerialDEBUG.println(" OK");
  flash(SUCCESSLED,4,100,100);

  //SerialDEBUG.print(F("Connecting to "));
  //SerialDEBUG.print(server);
  //SerialDEBUG.print("...");
  waiting(SUCCESSLED,ERRORLED,10);
  if (!client.connect(server, port)) {
      //SerialDEBUG.println(" fail");
      flash(ERRORLED,5,100,100);
      analogWrite(ERRORLED, 255);
      delay(2000);
    return;
  }
  stoping(ERRORLED,SUCCESSLED,INFOLED);
  //SerialDEBUG.println(" OK");
  flash(SUCCESSLED,5,100,100);
  //SerialDEBUG.print(F("Read txt file line by line and send data to web server: "));
  
  //Read only last 144 (24*6) lines (data for a last day)
  //SerialDEBUG.println("File length: "+ String (daily_file.size()));
  //SerialDEBUG.println("Set new file position");
  daily_file.seek(daily_file.size()-15676); //15676 simbols, approx 24*6=144 lines

  while (daily_file.available()) {

    data_string = daily_file.readStringUntil('\n');
    data_string = daily_file.readStringUntil('\n');
    data_string.trim();
    //SerialDEBUG.println("Readed line: ");
    //SerialDEBUG.println(data_string); //Printing for debugging purpose         
    // Make a HTTP POST request:
    starting(ERRORLED,SUCCESSLED,INFOLED);

    //SerialDEBUG.println("Create and send post request: ");
    //SerialDEBUG.println("POST " + String (resource) + " HTTP/1.1\r\nHost: "+ String (server) + "\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length:" + String (data_string.length()) +"\r\n\r\n"+ String (data_string));
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
    char a[300];
    while (client.connected() && millis() - timeout < 10000L) {
      // Create massive available data  
      int i;
      i=0;
      while (client.available()) {
        char c = client.read();
        a[i]= c;
        i++;        
        timeout = millis();
        analogWrite(SUCCESSLED, i);
       
    }
    
    }
    //get http response status code
    status_code=String(a[9])+String(a[10])+String(a[11]);
    
    if(status_code.equals(HTTP_OK_STATUS)) { 
      //SerialDEBUG.println("Succesufuly write data to influx!"); 
      analogWrite(SUCCESSLED, 255);
      analogWrite(ERRORLED, 0);
      analogWrite(INFOLED, 255); 
      delay(1000);    
    }
    else { 
      //SerialDEBUG.println("Not Succesufuly write data to influx!");
      //SerialDEBUG.println(a);
      flash(ERRORLED,5,100,100);
      analogWrite(ERRORLED, 255);
      delay(2000);
      return;
    }
    
    stoping(ERRORLED,SUCCESSLED,INFOLED);
  }

  client.stop();
  //SerialDEBUG.println("Server disconnected");
  flash(SUCCESSLED,1,100,100);

  modem.gprsDisconnect();
  //SerialDEBUG.println("GPRS disconnected");

  flash(SUCCESSLED,1,100,100);


  //turn on 1-st arduino
  digitalWrite(8, HIGH);

  //SerialDEBUG.println("turn on 1-st arduino");

  //turn off power gprs shild via mosfet
  digitalWrite(9, LOW);
 
   //SerialDEBUG.println("turn off power gprs shild via mosfet");

  //sleep for a 24 hour //turn off sd card pins
  //SerialDEBUG.println("Wait for while");
  while(1);
  
}
   


void flash (char led, char count, unsigned short interval, unsigned short pause)
{ 
  char i;
  pinMode(led, OUTPUT);
  
  for (i=0; i < count; i++) {
      analogWrite(led, 255);
      delay(interval);
      analogWrite(led, 0);
      delay(pause);     
    }
  
 
  }


void stoping (int led1,int led2, int led3)
{ 
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  analogWrite(led1, 0);
  analogWrite(led2, 0);
  analogWrite(led3, 0);
    
}

void starting (int led1,int led2,int led3)
{ 
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  analogWrite(led1, 255);
  analogWrite(led2, 255);
  analogWrite(led3, 255);
    
}

void waiting (int led1,int led2, unsigned short interval) { 
  fadein(led1,interval);
  fadein(led2,interval);
  fadeout(led1,interval);
  fadeout(led2,interval);

}

void fadein (int led, unsigned short interval) {

  pinMode(led, OUTPUT);  
  for (int i=0; i<255; i++) {
    analogWrite(led, i);
    delay (interval);
  }
  
}

void fadeout (int led, unsigned short interval) {
  pinMode(led, OUTPUT);
  for (int i=255; i>=0; i--) {
    analogWrite(led, i);
    delay (interval);
  }
  
}

  
