#include <Wire.h>
#include "SevSeg.h"

#define SLAVE_ADDR 0x09
#define BTN_PIN 2

// We're measuring seconds as the most granular unit of time
unsigned seconds = 0;
SevSeg seg;

void setup() {
  Serial.begin(9600);
  Serial.println("Slave");

  byte numDigits = 4;
  byte digitPins[] = {10, 11, 12, 13};
  byte segmentPins[] = {9, A3, 3, 5, 6, 8, 7, 4};

  bool resistorsOnSegments = true;
  bool updateWithDelaysIn = true;
  byte hardwareConfig = COMMON_CATHODE;
  seg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments);
  seg.setBrightness(90);

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
}

void btn_pressed()
{
}

enum class Button {
  power, volUp, funcStop, skipBack, play, skipForward,
  down, volDown, up, zero, eq, stRept, one, two, three,
  four, five, six, seven, eight, nine, repeat
};
Button input;

void recvEvent(int bytes) {
  while(Wire.available()) {
    input = (Button)Wire.read();
  }
}

unsigned makeDisplayValue() {
  unsigned minutes = seconds % 60;
  unsigned hours = (seconds/60) % 60;

  return (hours * 100) + (minutes);
}

enum class State {setTime, displayTime};
State state = State::displayTime;
void loop() {
  if (input == Button::one)
    state = State::displayTime;
  else if (input == Button::two)
    state = State::setTime;
  switch (state) {
    case State::displayTime:
      seg.setNumber(makeDisplayValue(), 2);
      break;
    case State::setTime:
      if (seconds % 2 == 0)
        seg.blank();
      else
        seg.setNumber(0, 2);
      break;
  }
  seg.refreshDisplay();
}
