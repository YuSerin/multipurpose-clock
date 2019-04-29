#include <Wire.h>
#include "SevSeg.h"

#define SLAVE_ADDR 0x09
#define BTN_PIN 2
#define LED_PIN 0
#define SEC_PER_HOUR 3600

enum class Button {
  power, volUp, funcStop, skipBack, play, skipForward,
  down, volDown, up, zero, eq, stRept, one, two, three,
  four, five, six, seven, eight, nine, repeat, none
};

enum class State {
  setTime, displayTime
};

// We're measuring seconds as the most granular unit of time
long time = 0;
SevSeg seg;

// NOTE:
// Serial functions can't be used here because they'll interfere with usage
// of ports 0 and 1
void setup() {

  // Segment display init
  byte numDigits = 4;
  byte digitPins[] = {10, 11, 12, 13};
  byte segmentPins[] = {9, A3, 3, 5, 6, 8, 7, 4};
  bool resistorsOnSegments = true;
  bool updateWithDelaysIn = true;
  byte hardwareConfig = COMMON_CATHODE;
  seg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments);
  seg.setBrightness(90);

  // Button init
  pinMode(BTN_PIN, INPUT_PULLUP);
  attachInterrupt(0,btn_pressed,FALLING);

  // Serial comms init
  Wire.begin(SLAVE_ADDR);
  Wire.onReceive(recvEvent);

  // LED init
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize timer interrupt registers
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 15625; // (16MHz/1024) * 1s

  // Timer control register set Clock/1024 //
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS10);
  TCCR1B |= (1 << CS12);
  TIMSK1  = (1 << OCIE1A);
  sei();
}

ISR(TIMER1_COMPA_vect) {
  ++time;
  if (time == 86400) // 24 hours worth of seconds, rollover
    time = 0;
}

void btn_pressed()
{
}

Button input;

void recvEvent(int bytes) {
  while(Wire.available()) {
    input = (Button)Wire.read();
  }
}


// State builders
State state = State::setTime;

State fromDisplayTime(Button input) {
  switch (input) {
    case Button::one:
      return State::setTime;
    default:
      return State::displayTime;
  }
}

long calcSecondsByHHMMPlace(int place, int multiplier) {
  switch (place) {
    case 3: // Tens of hours
      return (10L * SEC_PER_HOUR * multiplier);
    case 2: // Hours
      return (SEC_PER_HOUR * multiplier);
    case 1: // Tens of minutes
      return (10L * 60 * multiplier);
    case 0: // Minutes
      return (60L * multiplier);
  }
}

// Stateful values for setting a new time
long newTime = 0;
int inputPlace = 3;

State fromSetTime(Button input) {

  int multiplier;
  switch (input) {
    case Button::zero:
      multiplier = 0;
      break;
    case Button::one:
      multiplier = 1;
      break;
    case Button::two:
      multiplier = 2;
      break;
    case Button::three:
      multiplier = 3;
      break;
    case Button::four:
      multiplier = 4;
      break;
    case Button::five:
      multiplier = 5;
      break;
    case Button::six:
      multiplier = 6;
      break;
    case Button::seven:
      multiplier = 7;
      break;
    case Button::eight:
      multiplier = 8;
      break;
    case Button::nine:
      multiplier = 9;
      break;
    default:
      return State::setTime;
  }

  newTime += calcSecondsByHHMMPlace(inputPlace, multiplier);

  // Done inputting
  if (inputPlace == 0) {
    time = newTime;

    // Reset stateful values for next time
    inputPlace = 3;
    newTime = 0;
    return State::displayTime;
  }
  else {
    --inputPlace;
    return State::setTime;
  }
}

// Display builders
void secondsToHHMM(long seconds, char* buffer) {
  long minutes = (seconds/60) % 60;
  long hours = (seconds/SEC_PER_HOUR) % 24;
  buffer[3] = String(minutes % 10)[0];
  buffer[2] = String(minutes / 10)[0];
  buffer[1] = String(hours % 10)[0];
  buffer[0] = String(hours / 10)[0];
}

void fromDisplayTime(State newState) {
  char hhmm[5];
  secondsToHHMM(time, hhmm);
  seg.setChars(hhmm);
}

void fromSetTime(State newState) {
  if (time % 2 == 0)
    seg.blank();
  else {
    char hhmm[5];
    secondsToHHMM(newTime, hhmm);
    seg.setChars(hhmm);
  }
}

State buildNewState(State lastState, Button input) {
  switch (lastState) {
    case State::displayTime: return fromDisplayTime(input);
    case State::setTime:     return fromSetTime(input);
  }
}

void display(State newState) {
  switch (newState) {
    case State::displayTime: fromDisplayTime(newState); break;
    case State::setTime:     fromSetTime(newState); break;
  }
  seg.refreshDisplay();
}

void loop() {

  // Get new input for processing and then clear the global value to indicate
  // that it's been processed (like a queue with capacity of one)
  Button newInput = input;
  input = Button::none;

  // Build new state based on previous state and new input
  state = buildNewState(state, newInput);

  // Rebuild display based on new state
  display(state);
}
