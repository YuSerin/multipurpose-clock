#include <IRremote.h>
#include <Wire.h>
#include <LiquidCrystal.h>

#define SLAVE_ADDR 0x09
#define RECV_PIN A2

IRrecv irrecv(RECV_PIN);
decode_results results;

enum class Button {
  power, volUp, funcStop, skipBack, play, skipForward,
  down, volDown, up, zero, eq, stRept, one, two, three,
  four, five, six, seven, eight, nine, repeat
};

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
int menuPage;

Button getButtonRepresentation(int rawValue) {
    switch (rawValue) {
      case 0xffa25d:   Serial.println("power");        return Button::power;
      case 0xffe21d:   Serial.println("func/stop");    return Button::funcStop;
      case 0xff629d:   Serial.println("vol+");         return Button::volUp;
      case 0xff22dd:   Serial.println("fast back");    return Button::skipBack;
      case 0xff02fd:   Serial.println("pause");        return Button::play;
      case 0xffc23d:   Serial.println("fast forward"); return Button::skipForward;
      case 0xffe01f:   Serial.println("down");         return Button::down;
      case 0xffa857:   Serial.println("vol-");         return Button::volDown;
      case 0xff906f:   Serial.println("up");           return Button::up;
      case 0xff9867:   Serial.println("eq");           return Button::eq;
      case 0xffb04f:   Serial.println("st/rept");      return Button::stRept;
      case 0xff6897:   Serial.println("0");            return Button::zero;
      case 0xff30cf:   Serial.println("1");            return Button::one;
      case 0xff18e7:   Serial.println("2");            return Button::two;
      case 0xff7a85:   Serial.println("3");            return Button::three;
      case 0xff10ef:   Serial.println("4");            return Button::four;
      case 0xff38c7:   Serial.println("5");            return Button::five;
      case 0xff5aa5:   Serial.println("6");            return Button::six;
      case 0xff42bd:   Serial.println("7");            return Button::seven;
      case 0xff4ab5:   Serial.println("8");            return Button::eight;
      case 0xff52ad:   Serial.println("9");            return Button::nine;
      case 0xffffffff: Serial.println("repeat");       return Button::repeat;
    }
}

void setup()
{
  Serial.begin(9600);

  Wire.begin(); // Connect to slave //
  irrecv.enableIRIn(); // Start the receiver

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // Print the first page of the menu.
  menuPage = 1;
  printMenu(menuPage);
}

void loop()
{
  if (irrecv.decode(&results))
    {
      Button input = getButtonRepresentation(results.value);
      transmit(input);
      switch (input){
        case Button::up:
          menuPage--;
          printMenu(menuPage);
          break;
        case Button::down:
          menuPage++;
          printMenu(menuPage);
          break;
      }
      delay(100);
      irrecv.resume(); // Receive the next value
    }
}

void transmit(Button button)
{
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write((int)button);
  Wire.endTransmission();
}

void printMenu(int menuPage)
{
  if(menuPage < 1)
    menuPage = 1;
  if(menuPage > 2)
    menuPage = 2;

  switch(menuPage){
    case 1:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("1: Set Time");
      lcd.setCursor(0,1);
      lcd.print("2: Set Alarm");
    break;
    case 2:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("3: ");
      lcd.setCursor(0,1);
      lcd.print("4: ");
    break;
  }
}
