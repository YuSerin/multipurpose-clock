#include <Wire.h>
#include "SevSeg.h"

#define SLAVE_ADDR 0x09
#define BTN_PIN 2
#define LED_PIN 0
#define SEC_PER_HOUR 3600
#define SEC_PER_DAY 86400

enum class IRInput {
  power, volUp, funcStop, skipBack, play, skipForward,
  down, volDown, up, zero, eq, stRept, one, two, three,
  four, five, six, seven, eight, nine, repeat, none
};

enum class State {
  setTime, displayTime, setAlarm, alarmTriggered
};

struct Clock {
  State state;
  long time; // We're measuring seconds as the most granular unit of time
  long newTime; // For setting a new time
  IRInput input; // Most recent unprocessed input
} clock; // Need a global instance of this for the interrupt handlers to use

SevSeg seg;

// NOTE:
// Serial functions can't be used here because they'll interfere with usage
// of ports 0 and 1
void setup() {

  // Initialize state
  clock.time = 0;
  clock.newTime = 0;
  clock.state = State::displayTime;
  clock.input = IRInput::none;

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
  ++clock.time;
  if (clock.time == SEC_PER_DAY)
    clock.time = 0;
}

void btn_pressed()
{
}


void recvEvent(int bytes) {
  while(Wire.available()) {
    clock.input = (IRInput)Wire.read();
  }
}


// State builders

void updateFromDisplayTime() {
  switch (clock.input) {
  case IRInput::one:
    clock.state = State::setTime;
    break;
  case IRInput::two:
    clock.state = State::setAlarm;
  default:
    return;
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

void updateFromSetTime() {

  static int inputPlace = 3;

  int multiplier;
  switch (clock.input) {
  case IRInput::zero:
    multiplier = 0;
    break;
  case IRInput::one:
    multiplier = 1;
    break;
  case IRInput::two:
    multiplier = 2;
    break;
  case IRInput::three:
    multiplier = 3;
    break;
  case IRInput::four:
    multiplier = 4;
    break;
  case IRInput::five:
    multiplier = 5;
    break;
  case IRInput::six:
    multiplier = 6;
    break;
  case IRInput::seven:
    multiplier = 7;
    break;
  case IRInput::eight:
    multiplier = 8;
    break;
  case IRInput::nine:
    multiplier = 9;
    break;
  default:
    return;
  }

  clock.newTime += calcSecondsByHHMMPlace(inputPlace, multiplier);

  // Done inputting - update time, reset relevant setTime values, and change state to display
  if (inputPlace == 0) {
    clock.time = clock.newTime;
    clock.newTime = 0;
    inputPlace = 3;
    clock.state = State::displayTime;
  }
  else --inputPlace;
}

// Dispatcher for building next state
void updateState() {
  switch (clock.state) {
  case State::displayTime:    updateFromDisplayTime(); break;
  case State::setTime:        updateFromSetTime(); break;
  // case State::setAlarm:       return fromSetAlarm(input);
  // case State::alarmTriggered: return State::alarmTriggered; // This state can only be left by the button interrupt
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

void displayHHMM(long seconds) {
  char hhmm[5];
  secondsToHHMM(seconds, hhmm);
  seg.setChars(hhmm);
}

void displayFromSetTime() {
  if (clock.time % 2 == 0)
    seg.blank();
  else {
    displayHHMM(clock.newTime);
  }
}

// Dispatcher for rendering next display frame
void display() {
  switch (clock.state) {
  case State::displayTime:    displayHHMM(clock.time); break;
  case State::setTime:        displayFromSetTime(); break;
  // case State::setAlarm:       displayHHMM(alarmTime); break;
  // case State::alarmTriggered: fromDisplayTime(newState); break;
  }
  seg.refreshDisplay();
}

// Main read-eval-print loop (though the reading is done async by the serial handler...)
void loop() {
  updateState();
  // Clear input now that it's been processed
  clock.input = IRInput::none;
  display();
}
