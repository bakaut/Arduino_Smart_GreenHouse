
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


// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "internet.mts.ru";
const char user[] = "mts";
const char pass[] = "mts";

File daily_file;

// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial

// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(3, 2); // RX, TX

TinyGsm modem(Serial);
TinyGsmClient client(modem);

const char server[] = "169.51.23.245";
const char resource[] = "/write?db=weather";
String data = "weather,location=uglovo,region=aerodrom temp=4.00,hum=10000,pressure=98633.00,delta=19 1513348650000000000";
const int port = 30000;
const int chipSelect = 10;
String data_string;


void setup() {
 pinMode(4, OUTPUT);
  // Set console baud rate
  //Serial.begin(9600);
  //delay(10);

  // Set GSM module baud rate
  SerialAT.begin(115200);
  delay(1000);

  //SerialAT.println("Turn on power gprs shild via mosfet");
   //turn on power gprs shild via mosfet
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  
  //SerialAT.println("Power on gsm-gprs shild");
  //power on gsm-gprs shild
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
  delay(2500);
  digitalWrite(7, LOW);
  delay(500);
  
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  //SerialAT.println(F("Initializing modem..."));
  //blinking(13,MINI,MINI,MINI,MINI,300);
  //waiting(2,3,200);
  
  if (!modem.restart()) {
      flash(3,1,100,100);
      digitalWrite(3, HIGH);
      delay(2000);
      return;
    }
  //stoping(2,3);
  
  flash(2,1,100,100);

//  String modemInfo = modem.getModemInfo();
  //Serial.print("Modem: ");
  //Serial.println(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");
}

void loop() {
  
 
 
  //turn off 1-st arduino
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);

  SerialAT.println(F("Initializing sd..."));
  SdFat SD;
 
  //init sd card
  if (!SD.begin(chipSelect)) {
    //blinking(13,MINI,MINI,MAXI,MAXI,PAUSE);
  // don't do anything more:
  //SerialAT.println(F("Initializing sd fail"));
  flash(3,1,100,100);
  digitalWrite(3, HIGH);
  delay(2000);
  return;
  }

  daily_file = SD.open("file.txt");

  if (!daily_file) {
    //SerialAT.print("The text file.tx file cannot be opened");
    flash(3,2,100,100);
    digitalWrite(3, HIGH);
    delay(2000);
    return;
  }
  flash(2,2,100,100);
  //blinking(13,MINI,MINI,MINI,MINI,300);

  //blinking(13,MINI,MINI,MINI,MINI,300);


  //Connect gprs
  //SerialAT.println(F("Waiting for network..."));
  //blinking(13,MINI,MINI,MINI,MINI,300);
  //flash(2,100,100);
  waiting(2,3,200);
  if (!modem.waitForNetwork()) {
    //SerialAT.println(" fail");
    //blinking(13,MINI,MINI,MAXI,MAXI,PAUSE);
      flash(3,3,100,100);
      digitalWrite(3, HIGH);
      delay(2000);
      return;
  }
   stoping(2,3);
  //SerialAT.println(" OK");
   flash(2,3,100,100);

  //SerialAT.print(F("Connecting to "));
  //SerialAT.print(apn);
  //blinking(13,MINI,MINI,MINI,MINI,300);
  waiting(2,3,200);
  if (!modem.gprsConnect(apn, user, pass)) {
    //SerialAT.println(" fail");
    //blinking(13,MINI,MINI,MAXI,MAXI,PAUSE);
      flash(3,4,100,100);
      digitalWrite(3, HIGH);
      delay(2000);
      return;
  }
  stoping(2,3);
  //SerialAT.println(" OK");
  flash(2,4,100,100);

  //SerialAT.print(F("Connecting to "));
  //SerialAT.print(server);
  //blinking(13,MINI,MINI,MINI,MINI,300);
  waiting(2,3,200);
  if (!client.connect(server, port)) {
    //SerialAT.println(" fail");
      flash(3,5,100,100);
      digitalWrite(3, HIGH);
      delay(2000);
    return;
  }
  stoping(2,3);
  //Serial.println(" OK");
  flash(2,5,100,100);
  //blinking(13,MINI,MINI,MINI,MINI,300);
  stoping(2,3);
  while (daily_file.available()) {
    data_string = daily_file.readStringUntil('\n');
    //SerialAT.println(data_string); //Printing for debugging purpose         
    //do some action here
      // Make a HTTP POST request:
  digitalWrite(2, HIGH);
  digitalWrite(3, HIGH);
  digitalWrite(4, HIGH);
  
  //client.print("POST " + String (resource) + " HTTP/1.1\r\nHost: "+ String (server) + "\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length:" + String (data.length()) +"\r\n\r\n"+ String (data));
  client.print(String("POST ") + resource + " HTTP/1.0\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.println(data_string.length()); 
  client.println(); 
  client.print(data_string);
  client.println();
  //client.print("Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.connected() && millis() - timeout < 10000L) {
    // Print available data
    while (client.available()) {
      char c = client.read();
   //   SerialAT.print(c);
   //flash(2,300,300);
      timeout = millis();
      flash(2,3,50,50);
    }
  }
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  delay(1000);
 // SerialAT.println();
  }
   

  client.stop();
  //SerialAT.println("Server disconnected");
  flash(2,1,100,100);
 // blinking(13,MINI,MINI,MINI,MINI,300);
  modem.gprsDisconnect();
  //SerialAT.println("GPRS disconnected");
  //blinking(13,MINI,MINI,MINI,MINI,300);
  flash(2,1,100,100);


  //turn on 1-st arduino
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  //blinking(13,MINI,MINI,MINI,MINI,300);

  //turn off power gprs shild via mosfet
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
  //blinking(13,MINI,MINI,MINI,MINI,300);
  
  //sleep for a 24 hour //turn off sd card pins
  while(1);

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
  
