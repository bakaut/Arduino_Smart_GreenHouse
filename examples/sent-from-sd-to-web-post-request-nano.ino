/**************************************************************
 *
 * This sketch connects to a website and downloads a page.
 * It can be used to perform HTTP/RESTful API calls.
 *
 * TinyGSM Getting Started guide:
 *   http://tiny.cc/tiny-gsm-readme
 *
 **************************************************************/


#define TINY_GSM_MODEM_A7
#define MINI 100
#define MAXI 700
#define PAUSE 500


#include <TinyGsmClient.h>
#include <SD.h>

// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "internet.mts.ru";
const char user[] = "mts";
const char pass[] = "mts";

File daily_file;

// Use Hardware Serial on Mega, Leonardo, Micro
//#define SerialAT Serial1

// or Software Serial on Uno, Nano
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(2, 3); // RX, TX

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

const char server[] = "169.51.23.245";
const char resource[] = "/write?db=weather";
String data = "weather,location=uglovo,region=aerodrom temp=4.00,hum=10000,pressure=98633.00,delta=19 1513348650000000000";
const int port = 30000;
const int chipSelect = 10;
String buffer;

void setup() {

  
  // Set console baud rate
  Serial.begin(115200);
  delay(10);

  // Set GSM module baud rate
  SerialAT.begin(9600);
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  //Serial.println(F("Initializing modem..."));
  blinking(13,MINI,MINI,MINI,MINI,300);
  modem.restart();

//  String modemInfo = modem.getModemInfo();
  //Serial.print("Modem: ");
  //Serial.println(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");
}

void loop() {
 
  //turn off 1-st arduino
  pinMode(8, OUTPUT);
  digitalWrite(7, LOW);

  //init sd card
  if (!SD.begin(chipSelect)) {
    blinking(13,MINI,MINI,MAXI,MAXI,PAUSE);
  // don't do anything more:
  return;
  } 

  daily_file = SD.open("23");

  if (!daily_file) {
    Serial.print("The text file cannot be opened");
    return;
  }

  blinking(13,MINI,MINI,MINI,MINI,300);
   //turn on power gprs shild via mosfet
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  
  blinking(13,MINI,MINI,MINI,MINI,300);
   
  //power on gsm-gprs shild
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
  delay(2500);
  digitalWrite(7, LOW);
  delay(500);

  //Connect gprs
    //Serial.print(F("Waiting for network..."));
  blinking(13,MINI,MINI,MINI,MINI,300);
  if (!modem.waitForNetwork()) {
    //Serial.println(" fail");
    blinking(13,MINI,MINI,MAXI,MAXI,PAUSE);
    delay(10000);
    return;
  }
  //Serial.println(" OK");

 // Serial.print(F("Connecting to "));
  //Serial.print(apn);
  blinking(13,MINI,MINI,MINI,MINI,300);
  if (!modem.gprsConnect(apn, user, pass)) {
    //Serial.println(" fail");
    blinking(13,MINI,MINI,MAXI,MAXI,PAUSE);
    delay(10000);
    return;
  }
  //Serial.println(" OK");

  //Serial.print(F("Connecting to "));
  //Serial.print(server);
  blinking(13,MINI,MINI,MINI,MINI,300);
  if (!client.connect(server, port)) {
    //Serial.println(" fail");
    blinking(13,MINI,MINI,MAXI,MAXI,PAUSE);
    delay(10000);
    return;
  }
  //Serial.println(" OK");
  blinking(13,MINI,MINI,MINI,MINI,300);
  
  while (daily_file.available()) {
    buffer = daily_file.readStringUntil('\n');
    //Serial.println(buffer); //Printing for debugging purpose         
    //do some action here
      // Make a HTTP POST request:
  //client.print("POST " + String (resource) + " HTTP/1.1\r\nHost: "+ String (server) + "\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length:" + String (data.length()) +"\r\n\r\n"+ String (data));
  client.print(String("POST ") + resource + " HTTP/1.0\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.println(buffer.length()); 
  client.println(); 
  client.print(buffer);
  client.println();
  //client.print("Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (client.connected() && millis() - timeout < 10000L) {
    // Print available data
    while (client.available()) {
      char c = client.read();
      Serial.print(c);
      timeout = millis();
    }
  }
  Serial.println();
  }
   

  client.stop();
  //Serial.println("Server disconnected");
  blinking(13,MINI,MINI,MINI,MINI,300);
  modem.gprsDisconnect();
  //Serial.println("GPRS disconnected");
  blinking(13,MINI,MINI,MINI,MINI,300);


  //turn on 1-st arduino
  pinMode(8, OUTPUT);
  digitalWrite(7, HIGH);
  blinking(13,MINI,MINI,MINI,MINI,300);

  //turn off power gprs shild via mosfet
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
  blinking(13,MINI,MINI,MINI,MINI,300);
  
  //sleep for a 24 hour
  

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
