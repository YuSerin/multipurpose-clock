#include <boarddefs.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <ir_Lego_PF_BitStreamEncoder.h>
#include <Wire.h>

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
//char remote_cmd[100];
//String remote_cmd;
//int remote_cmd;
//int x = 0;

void setup()
{
  Serial.begin(9600);
  
  Wire.begin(); // Connect to slave //
  irrecv.enableIRIn(); // Start the receiver
  //remote_cmd = 0;
  //x = 0;
  //Serial.println("Master");
}

void loop()
{
//  if(x < 100) {
//    Wire.beginTransmission(SLAVE_ADDR); // transmit to device #9
//  Wire.write("x is ");        // sends five bytes
//  Wire.write(x);              // sends one byte
//  Wire.endTransmission();    // stop transmitting
//  x++;
//  delay(1000);
//  }
  
  if (irrecv.decode(&results))
    {
     //Serial.println(String(results.value, HEX));
     //Serial.println(results.value);
     
     //remote_cmd = int(results.value);
     //Serial.println(remote_cmd);
     //delay(100);
     //String(results.value, HEX).toCharArray(remote_cmd, 100);
     //Serial.println(remote_cmd);
     switch(results.value) {
      case btn_1: transmit(set_time);
      break;
      case btn_2: transmit(set_alarm);
      break;
      case btn_3: transmit(set_timer);
      break;
      case btn_4: // Show temp on LCD screen //
      break;
      case btn_up: transmit(cmd_up);
      break;
      case btn_down: transmit(cmd_down);
      break;
     }
     delay(100);
     //transmit(remote_cmd);
     irrecv.resume(); // Receive the next value
    }
}

void transmit(int cmd) 
{
  Wire.beginTransmission(SLAVE_ADDR); 
  Wire.write(cmd);
  Wire.endTransmission();
}
