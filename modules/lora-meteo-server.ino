#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_SSD1306.h>

#define LORAFREQ 433E6
#define LEDDEBUG

Adafruit_SSD1306 display(4);
String stringTotal;

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.display();
  delay(1000);
  display.clearDisplay();

  ToOledPrint("Starting Lora ... ", "print", 0,5);

  if (!LoRa.begin(LORAFREQ)) {
    ToOledPrint("Starting LoRa failed!", "print", 0,5);
    while (1);
  }
  ToOledPrint("Lora started ", "print", 0,5);
}

void loop() {
  // try to parse packet
  ToOledPrint("try to parse  ", "print", 0,5);
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    ToOledPrint("Received packet '", "print", 0,10);

    // read packet
    ToOledPrint("read packet '", "print", 0,10);
    while (LoRa.available()) {
      stringTotal += (String((char)LoRa.read())); 
    }
    //check_data_prefix = "weather,location=uglovo,region=aerodrom ";
    //add check_data_prefix
    ToOledPrint(stringTotal, "print", 0,5);
    delay(120000);

    // print RSSI of packet
    ToOledPrint("' with RSSI ", "print", 0,10);
    ToOledPrint(String(LoRa.packetRssi()), "print", 0,10);
  }
}

void ToOledPrint(String text, String mode, int x, int y) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(x,y);
  if(mode == "print"){
    display.setTextColor(WHITE);
    display.println(text); 
    display.display(); 
  }
  else {
    
    display.setTextColor(BLACK);
    display.println(text);
    display.display();  
  }
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
}