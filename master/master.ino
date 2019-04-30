#include <IRremote.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <SimpleDHT.h>

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

//set pin for temperature and humidity sensor
int pinDHT11 = 2;
SimpleDHT11 dht11;
//variables for reading temperature and humidity
byte temperature = 0;
byte humidity = 0;
byte data[40] = {0};

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
  //DHT11 sampling rate is 1HZ
  if(millis() % 1000 == 0)
  {
    //read then print temperature and humidity next to the menu
    dht11.read(pinDHT11, &temperature, &humidity, data);
    lcd.setCursor(16,0); lcd.print(" ");
    lcd.setCursor(16,1); lcd.print(" ");
    lcd.setCursor(12, 0); lcd.print(temperature); lcd.print("*C");
    lcd.setCursor(12, 1); lcd.print(humidity); lcd.print("%");

    //reset cursor
    lcd.setCursor(0,0);
    Serial.println("bep");
  }
  
  if (irrecv.decode(&results))
    {
      Button input = getButtonRepresentation(results.value);
      transmit(input);
      switch (input)
      {
        case Button::up:
          menuPage--;
          printMenu(menuPage);
          break;
        case Button::down:
          menuPage++;
          printMenu(menuPage);
          break;
        case Button::one:
          menuPage = 1;
          setTheTime();
          printMenu(menuPage);
          break;
        case Button::two:
          menuPage = 1;
          setTheAlarm();
          printMenu(menuPage);
          break;
      }
      delay(100);
      irrecv.resume(); // Receive the next value
    }
}

//sends pressed button representation to slave arduino
void transmit(Button button)
{
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write((int)button);
  Wire.endTransmission();
}

//prints the menu to the LCD based on which page the user is on,
//which is determined by the up/down buttons.
void printMenu(int menuPage)
{
  //error check menu pages
  if(menuPage < 1)
    menuPage = 1;
  if(menuPage > 2)
    menuPage = 2;

  //switch to print menu options depending on page
  switch(menuPage){
    case 1:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("1:Set Time");
      lcd.setCursor(0,1);
      lcd.print("2:Set Alarm");
    break;
    case 2:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("3:");
      lcd.setCursor(0,1);
      lcd.print("4:");
    break;
  }
}

//puts the master arduino into set time mode while the slave arduino is
//so weird things don't happen
//also prints instructions to LCD to let user know that it is in this mode
void setTheTime()
{
  //so it doesn't send 1 as an input
  delay(200);
  irrecv.resume();
  //counter for button recieve count
  int count = 0;
  //print instructions
  printSetTimeInstructions();
  //loop for set time mode, ends after 4 button presses
  //ASSUMPTION: The user will enter 4 digits and not press any other buttons!!!
  while(count != 4)
  {
    //if a button is pressed
    if (irrecv.decode(&results))
    {
      //interpret and send to slave for setting time
      Button input = getButtonRepresentation(results.value);
      transmit(input);
      //increment the counter because a button was pressed
      count++;
      //get ready to recieve another button press
      delay(100);
      irrecv.resume();
    }
  }
}

//prints instructions to LCD to signal that user is in set time mode
void printSetTimeInstructions()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set time new 24-");
  lcd.setCursor(0,1);
  lcd.print("hour time");
}

//puts the master arduino into set alarm mode while the slave arduino is
//so weird things don't happen
//also prints instructions to LCD to let user know that it is in this mode
void setTheAlarm()
{
  //so it doesn't send 1 as an input
  delay(200);
  irrecv.resume();
  //counter for button recieve count
  int count = 0;
  //print instructions
  printSetTimeInstructions();
  //loop for set alarm mode, ends after 4 button presses
  //ASSUMPTION: The user will enter 4 digits and not press any other buttons!!!
  while(count != 4)
  {
    //if a button is pressed
    if (irrecv.decode(&results))
    {
      //interpret and send to slave for setting time
      Button input = getButtonRepresentation(results.value);
      transmit(input);
      //increment the counter because a button was pressed
      count++;
      //get ready to recieve another button press
      delay(100);
      irrecv.resume();
    }
  }
}

//prints instructions to LCD to signal that user is in set alarm mode
void printSetAlarmInstructions()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set alarm with");
  lcd.setCursor(0,1);
  lcd.print("24-hour time");
}
