#include <Wire.h>
#include "SevSeg.h"

#define SLAVE_ADDR 0x09
#define BTN_PIN 2

enum cmd {set_time, set_alarm, set_timer, cmd_up, cmd_down};
SevSeg sevseg;

void setup() 
{
  Serial.begin(9600);
  Serial.println("Slave");

  byte numDigits = 4;
  byte digitPins[] = {10, 11, 12, 13};
  byte segmentPins[] = {9, A3, 3, 5, 6, 8, 7, 4};

  bool resistorsOnSegments = true; 
  bool updateWithDelaysIn = true;
  byte hardwareConfig = COMMON_CATHODE; 
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments);
  sevseg.setBrightness(90);

  pinMode(BTN_PIN, INPUT_PULLUP);
  attachInterrupt(0,btn_pressed,FALLING);
  
  Wire.begin(SLAVE_ADDR);
  Wire.onReceive(recvEvent);
}

void btn_pressed() 
{
  
}

void recvEvent(int bytes) 
{  
  while(Wire.available()) {
    int cmd = Wire.read();
    Serial.print(cmd);         // print the character
  }
}

void loop() 
{
  sevseg.setNumber(0, 2);
  sevseg.refreshDisplay();
  delay(100);
}
