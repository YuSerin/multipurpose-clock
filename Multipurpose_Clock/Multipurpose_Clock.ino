#include "SevSeg.h"
#define hour_btn A1
#define min_btn A0

bool hr_btn_pressed;
bool min_btn_pressed;
int hour;
int minute;

SevSeg sevseg;

void setup() {
  Serial.begin(9600);
  byte numDigits = 4;
  byte digitPins[] = {10, 11, 12, 13};
  byte segmentPins[] = {9, 2, 3, 5, 6, 8, 7, 4};

  bool resistorsOnSegments = true; 
  bool updateWithDelaysIn = true;
  byte hardwareConfig = COMMON_CATHODE; 
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments);
  sevseg.setBrightness(90);
  sevseg.setChars("----");
  delay(1000);

  pinMode(hour_btn, INPUT_PULLUP);
  pinMode(min_btn, INPUT_PULLUP);
  hr_btn_pressed = false;
  min_btn_pressed = false;
  hour = 0;
  minute = 0;
}

void loop() {
  sevseg.setNumber(hour + minute, 2);  // Show set time //
  
  // Set hour: //
  if(digitalRead(hour_btn) == LOW && !hr_btn_pressed) {
    hr_btn_pressed = true;
    hour += 100;
    //Serial.println(hour);
    if(hour > 2400) {  // Military time //
      hour = 100;  // Reset to 1:00 //
    }
  }
  else if(digitalRead(hour_btn) == HIGH && hr_btn_pressed) {
    hr_btn_pressed = false;
  }

  // Set Minute: //
  if(digitalRead(min_btn) == LOW && !min_btn_pressed) {
    min_btn_pressed = true;
    minute++;
    //Serial.println(minute);
    if(minute > 59) {  // Military time //
      minute = 0;
    }
  }
  else if(digitalRead(min_btn) == HIGH && min_btn_pressed) {
    min_btn_pressed = false;
  }
  sevseg.refreshDisplay();
}
