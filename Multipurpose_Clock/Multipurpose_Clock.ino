#include "SevSeg.h"

#define BTN_PIN 2

SevSeg sevseg;

void setup() {
  Serial.begin(9600);
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
}

void btn_pressed() {
  
}

void loop() {
  //sevseg.setNumber(hour + minute, 2);  // Show set time //
  sevseg.setNumber(0, 2);
  
  // Set hour: //
//  if(digitalRead(hour_btn) == LOW && !hr_btn_pressed) {
//    hr_btn_pressed = true;
//    hour += 100;
//    //Serial.println(hour);
//    if(hour > 2400) {  // Military time //
//      hour = 100;  // Reset to 1:00 //
//    }
//  }
//  else if(digitalRead(hour_btn) == HIGH && hr_btn_pressed) {
//    hr_btn_pressed = false;
//  }
//
//  // Set Minute: //
//  if(digitalRead(min_btn) == LOW && !min_btn_pressed) {
//    min_btn_pressed = true;
//    minute++;
//    //Serial.println(minute);
//    if(minute > 59) {  // Military time //
//      minute = 0;
//    }
//  }
//  else if(digitalRead(min_btn) == HIGH && min_btn_pressed) {
//    min_btn_pressed = false;
//  }
  sevseg.refreshDisplay();
}
