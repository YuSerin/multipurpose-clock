#include <Wire.h>
#include "SevSeg.h"

#define SLAVE_ADDR 0x09
#define BTN_PIN 2
#define LED_PIN 0
#define BUZZER_PIN 1
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
  long alarm;
  IRInput input; // Most recent unprocessed input
  bool alarmEnabled;
} clock; // Need a global instance of this for the interrupt handlers to use

SevSeg seg;

// NOTE:
// Serial functions can't be used here because they'll interfere with usage
// of ports 0 and 1
void setup() {

  // Initialize state
  // TODO: Revert these
  // clock.time = 0;
  clock.time = 60;
  clock.newTime = 0;
  clock.state = State::displayTime;
  clock.input = IRInput::none;
  // clock.alarm = 0;
  // clock.alarmEnabled = false;
  clock.alarm = 120;
  clock.alarmEnabled = true;

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
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), buttonPressed, FALLING);

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
  // TCCR1B |= (1 << CS10);
  TCCR1B |= (1 << CS12);
  TIMSK1  = (1 << OCIE1A);
  sei();
}

ISR(TIMER1_COMPA_vect) {
  ++clock.time;
  if (clock.time == SEC_PER_DAY)
    clock.time = 0;
}

void buttonPressed() {
  if (clock.state == State::alarmTriggered) {
    clock.state = State::displayTime;
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
  }
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
  case IRInput::power:
    clock.alarmEnabled = !clock.alarmEnabled;
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

int getMultiplierFromInput() {

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
    return -1; // Signifies that a non-numeric button was pressed and should be ignored
  }
  return multiplier;
}

void updateFromSetTime() {

  static int inputPlace = 3;

  int multiplier = getMultiplierFromInput();
  if (multiplier == -1) return; // Invalid input - ignore it

  clock.newTime += calcSecondsByHHMMPlace(inputPlace, multiplier);

  // Done inputting - update time, reset relevant values, and change state to display
  if (inputPlace == 0) {
    clock.time = clock.newTime;
    clock.newTime = 0;
    inputPlace = 3;
    clock.state = State::displayTime;
  }
  else --inputPlace;
}

void updateFromSetAlarm() {

  static int inputPlace = 3;

  if (inputPlace == 3)
    clock.alarm = 0; // Reset to zero in case it was previously set

  int multiplier = getMultiplierFromInput();
  if (multiplier == -1) return; // Invalid input - ignore it

  clock.alarm += calcSecondsByHHMMPlace(inputPlace, multiplier);

  // Done inputting - set alarm, reset relevant values, and change state to display
  if (inputPlace == 0) {
    inputPlace = 3;
    clock.state = State::displayTime;
    clock.alarmEnabled = true;
  }
  else --inputPlace;
}

// Dispatcher for building next state
void updateState() {
  // Check for triggers
  if (clock.alarmEnabled && clock.time == clock.alarm) {
    clock.state = State::alarmTriggered;
    return;
  }
  switch (clock.state) {
  case State::displayTime:    updateFromDisplayTime(); break;
  case State::setTime:        updateFromSetTime(); break;
  case State::setAlarm:       updateFromSetAlarm(); break;
  // case State::alarmTriggered: return State::alarmTriggered; // This state can only be left by the button interrupt
  }
}

// Display builders
void secondsToHHMM(long seconds, char* buffer) {
  long minutes = (seconds/60) % 60;
  long hours = (seconds/SEC_PER_HOUR) % 24;
  buffer[0] = String(hours / 10)[0];
  buffer[1] = String(hours % 10)[0];
  buffer[2] = String(minutes / 10)[0];
  buffer[3] = String(minutes % 10)[0];
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

void displayFromAlarmTriggered() {
  if (clock.time % 2 == 0) {
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
  }
  else {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 440);
    displayHHMM(clock.time);
  }
}

// Dispatcher for rendering next display frame
void display() {
  switch (clock.state) {
  case State::displayTime:    displayHHMM(clock.time); break;
  case State::setTime:        displayFromSetTime(); break;
  case State::setAlarm:       displayHHMM(clock.alarm); break;
  case State::alarmTriggered: displayFromAlarmTriggered(); break;
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
