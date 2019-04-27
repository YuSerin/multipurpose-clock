#include <boarddefs.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <ir_Lego_PF_BitStreamEncoder.h>
#include <Wire.h>
#include<LiquidCrystal.h>

#define SLAVE_ADDR 0x09
#define RECV_PIN A2
#define btn_1 16724175  // Set Time //
#define btn_2 16718055  // Set Alarm //
#define btn_3 16743045  // Set Timer //
#define btn_4 16716015  // Show Temperature //
#define btn_5 16726215
#define btn_down 16769055 
#define btn_up 16748655

enum cmd {set_time, set_alarm, set_timer, cmd_up, cmd_down};

IRrecv irrecv(RECV_PIN);
decode_results results;


LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
int menuPage;

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
      Serial.println(results.value);
     switch(results.value) {
      case btn_1: transmit(set_time);
      break;
      case btn_2: transmit(set_alarm);
      break;
      case btn_3: transmit(set_timer);
      break;
      case btn_4: // Show temp on LCD screen //
      break;
      case btn_up: menuPage--; printMenu(menuPage); transmit(cmd_up);
      break;
      case btn_down: menuPage++; printMenu(menuPage); transmit(cmd_down);
      break;
     }
     delay(100);
     irrecv.resume(); // Receive the next value
    }
}

void transmit(int cmd) 
{
  Wire.beginTransmission(SLAVE_ADDR); 
  Wire.write(cmd);
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
