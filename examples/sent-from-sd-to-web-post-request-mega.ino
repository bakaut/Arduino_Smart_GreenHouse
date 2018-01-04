
/**************************************************************
 *
 * This sketch connects to a website and downloads a page.
 * It can be used to perform HTTP/RESTful API calls.
 *
 * TinyGSM Getting Started guide:
 *   http://tiny.cc/tiny-gsm-readme
 * !! SD FAT VERY SMALL! 232 byte. SD 852 byte!
 **************************************************************/

#define TINY_GSM_MODEM_A7
#define MINI 100
#define MAXI 700
#define PAUSE 500

#include <TinyGsmClient.h>

#include <BlockDriver.h>
#include <FreeStack.h>
#include <MinimumSerial.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <SysCall.h> 

//#include <SD.h>

//#include <stdio.h>
//#include <string.h>

// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "internet.mts.ru";
const char user[] = "mts";
const char pass[] = "mts";

File daily_file;

// Use Hardware Serial on Mega, Leonardo, Micro
//#define SerialAT Serial
#define SerialDEBUG Serial
#define SerialAT Serial1

// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(2, 3); // RX, TX

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

const char server[] = "169.51.23.245";
const char resource[] = "/write?db=weather";
String data = "weather,location=uglovo,region=aerodrom temp=4.00,hum=10000,pressure=98633.00,delta=19 1513348650000000000";
const int port = 30000;
const int chipSelect = 53;
String data_string;


void setup() {
  pinMode(4, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(53, OUTPUT);
  // Set console baud rate
  //Serial.begin(9600);
  //delay(10);


  
  SerialDEBUG.begin(115200);
  delay(1000);
  //SerialDEBUG.listen();

  // Set GSM module baud rate
  SerialAT.begin(115200);
  delay(1000);

  //SerialAT.println("Turn on power gprs shild via mosfet");
  //turn on power gprs shild via mosfet

  SerialDEBUG.print("Turn on power gprs shild via mosfet...");
  digitalWrite(9, HIGH);
  SerialDEBUG.println("Done");
  
  SerialDEBUG.print("Power on gsm-gprs shild...");
  //power on gsm-gprs shild
  
  digitalWrite(7, HIGH);
  delay(2500);
  digitalWrite(7, LOW);
  delay(500);
  SerialDEBUG.println("Done");
  
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  //SerialAT.println(F("Initializing modem..."));
  //blinking(13,MINI,MINI,MINI,MINI,300);
  waiting(5,6,200);

  
  SerialDEBUG.print("Restarting modem...");
  
  if (!modem.restart()) {
      SerialDEBUG.print("Failed");
      flash(5,1,100,100);
      digitalWrite(3, HIGH);
      delay(2000);
      return;
    }

  stoping(6,5);
  //SerialDEBUG.listen();
  SerialDEBUG.println("OK");
  flash(6,1,100,100);

//  String modemInfo = modem.getModemInfo();
  //Serial.print("Modem: ");
  //Serial.println(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");
}

void loop() {
  //SerialDEBUG.listen();
  SerialDEBUG.println("Turn off 1-st arduino");
  
  //turn off 1-st arduino
  
  digitalWrite(8, LOW);

  SerialDEBUG.print(F("Initializing sd..."));
  SdFat SD;
 
  //init sd card
  if (!SD.begin(chipSelect)) {
    //blinking(13,MINI,MINI,MAXI,MAXI,PAUSE);
  // don't do anything more:
  SerialDEBUG.println(F("Failed"));
  flash(5,1,100,100);
  digitalWrite(3, HIGH);
  delay(2000);
  return;
  }
  SerialDEBUG.println("Done");
  SerialDEBUG.print("Open text file...");
  
  daily_file = SD.open("file.txt", FILE_READ);
  
  if (!daily_file) {
    SerialDEBUG.print("The text file.txt file cannot be opened");
    flash(5,2,100,100);
    digitalWrite(3, HIGH);
    delay(2000);
    return;
  }
  SerialDEBUG.println("Done");
  flash(6,2,100,100);
  //blinking(13,MINI,MINI,MINI,MINI,300);

  //blinking(13,MINI,MINI,MINI,MINI,300);


  //Connect gprs
  SerialDEBUG.print(F("Waiting for network..."));
  //blinking(13,MINI,MINI,MINI,MINI,300);
  //flash(6,100,100);
  waiting(5,6,200);
  if (!modem.waitForNetwork()) {
    SerialDEBUG.println("Failed");
    //blinking(13,MINI,MINI,MAXI,MAXI,PAUSE);
      flash(5,3,100,100);
      digitalWrite(3, HIGH);
      delay(2000);
      return;
  }
   stoping(6,5);
   SerialDEBUG.println(" OK");
   flash(6,3,100,100);

  SerialDEBUG.print(F("Connecting to "));
  SerialDEBUG.print(apn);
  SerialDEBUG.print("...");
  //blinking(13,MINI,MINI,MINI,MINI,300);
  waiting(5,6,200);
  if (!modem.gprsConnect(apn, user, pass)) {
     SerialDEBUG.println(" fail");
    //blinking(13,MINI,MINI,MAXI,MAXI,PAUSE);
      flash(5,4,100,100);
      digitalWrite(3, HIGH);
      delay(2000);
      return;
  }
  stoping(6,5);
  SerialDEBUG.println(" OK");
  flash(6,4,100,100);

  SerialDEBUG.print(F("Connecting to "));
  SerialDEBUG.print(server);
  SerialDEBUG.print("...");
  //blinking(13,MINI,MINI,MINI,MINI,300);
  waiting(5,6,200);
  if (!client.connect(server, port)) {
      SerialDEBUG.println(" fail");
      flash(5,5,100,100);
      digitalWrite(3, HIGH);
      delay(2000);
    return;
  }
  stoping(6,5);
  SerialDEBUG.println(" OK");
  flash(6,5,100,100);
  //blinking(13,MINI,MINI,MINI,MINI,300);
  stoping(6,5);
  SerialDEBUG.print(F("Read txt file line by line and send data to web server: "));
  while (daily_file.available()) {
    data_string = daily_file.readStringUntil('\n');
    data_string.trim();
    SerialDEBUG.println("Readed line: ");
    SerialDEBUG.println(data_string); //Printing for debugging purpose         
    //do some action here
      // Make a HTTP POST request:
  digitalWrite(6, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(4, HIGH);

  SerialDEBUG.println("Create and send post request: ");
  SerialDEBUG.println("POST " + String (resource) + " HTTP/1.1\r\nHost: "+ String (server) + "\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length:" + String (data_string.length()) +"\r\n\r\n"+ String (data_string));
  client.print(String("POST ") + resource + " HTTP/1.1\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.println(data_string.length()); 
  client.println(); 
  client.print(data_string);
  client.println();
  //client.println();
  //client.print("Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  //char output[1000];
  while (client.connected() && millis() - timeout < 10000L) {
    // Print available data
    while (client.available()) {
      char c = client.read();
      SerialDEBUG.print(c);

      //strcpy(output,"Begin\r\n");
      //strcpy(output,c);
      //SerialDEBUG.print(output);
      //SerialDEBUG.print(c);
   //flash(6,300,300);
      timeout = millis();
      //flash(6,3,50,50);
    }
    //SerialDEBUG.println(output);
    SerialDEBUG.println();
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(4, LOW);
  delay(1000);
  }

  SerialDEBUG.println();

  client.stop();
  SerialDEBUG.println("Server disconnected");
  flash(6,1,100,100);
 // blinking(13,MINI,MINI,MINI,MINI,300);
  modem.gprsDisconnect();
  SerialDEBUG.println("GPRS disconnected");
  //blinking(13,MINI,MINI,MINI,MINI,300);
  flash(6,1,100,100);


  //turn on 1-st arduino
  digitalWrite(8, HIGH);
  //blinking(13,MINI,MINI,MINI,MINI,300);
  SerialDEBUG.println("turn on 1-st arduino");

  //turn off power gprs shild via mosfet
  digitalWrite(9, LOW);
  //blinking(13,MINI,MINI,MINI,MINI,300);
   SerialDEBUG.println("turn off power gprs shild via mosfet");

  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, HIGH);
  //sleep for a 24 hour //turn off sd card pins
  SerialDEBUG.println("Wait for while");
  
  while(1);
  
  }
}
   


//void blinking (char led, unsigned short a, unsigned short b, unsigned short c, unsigned short d, unsigned short pause)
//{ 
//  pinMode(led, OUTPUT);
//  flash (led,a,pause);
//  flash (led,b,pause);
//  flash (led,c,pause);
//  flash (led,d,pause);
//}

void flash (char led, char count, unsigned short interval, unsigned short pause)
{ 
  char i;
  pinMode(led, OUTPUT);
  
  for (i=0; i < count; i++) {
      digitalWrite(led, HIGH);
      delay(interval);
      digitalWrite(led, LOW);
      delay(pause);     
    }
  
 
  }

void waiting (char led1,char led2, unsigned short interval)
{ 
  char i;
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  for (i=0; i < 10; i++) {
     digitalWrite(led1, HIGH);
      digitalWrite(led2, HIGH);
      delay(interval);
      digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);
    
    }
  
 
  }

void stoping (char led1,char led2)
{ 
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
    
}
  
