#include <Wire.h>
#include "SevSeg.h"

#define SLAVE_ADDR 0x09
#define BTN_PIN 2

enum cmd {set_time, set_alarm, set_timer, cmd_up, cmd_down};
SevSeg sevseg;

// We're measuring seconds as the most granular unit of time
unsigned seconds = 0;

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

  // Initialize timer interrupt registers
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 15625; // (16MHz/1024) * 1s

  // Timer control register set Clock/1024 //
  TCCR1B |= (1 << WGM12);
  // TCCR1B |= (1 << CS10); // TODO: Uncomment for seconds
  TCCR1B |= (1 << CS12);
  TIMSK1  = (1 << OCIE1A);
  sei();
}

ISR(TIMER1_COMPA_vect) {
  ++seconds;
  Serial.print(seconds); Serial.print(" seconds\n");
}

void btn_pressed()
{
}

void recvEvent(int bytes) {
  while(Wire.available()) {
    int cmd = Wire.read();
    Serial.print(cmd);         // print the character
  }
}

unsigned make_display_value() {
  unsigned minutes = seconds % 60;
  unsigned hours = (seconds/60) % 60;

  return (hours * 100) + (minutes);
}

void loop()
{
  sevseg.setNumber(make_display_value(), 2);
  sevseg.refreshDisplay();
}
