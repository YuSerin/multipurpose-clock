#include <Wire.h>

#define SLAVE_ADDR 0x09

enum cmd {set_time, set_alarm, set_timer, cmd_up, cmd_down};

void setup() 
{
  Serial.begin(9600);
  Serial.println("Slave");
  
  Wire.begin(SLAVE_ADDR);
  Wire.onReceive(recvEvent);
}

void recvEvent(int bytes) {
  
  while(Wire.available()) {
    //char c = Wire.read(); // receive byte as a character
    int cmd = Wire.read();
    Serial.print(cmd);         // print the character
  }
//  while (1 < Wire.available()) { // loop through all but the last
//    char c = Wire.read(); // receive byte as a character
//    Serial.print(c);         // print the character
//  }
//  int x = Wire.read();    // receive byte as an integer
//  Serial.println(x);         // print the integer
  //for (int i = 0; i < bytes; ++i) {
    //remote_cmd[0] = Wire.read();
  //}
  //Serial.println(remote_cmd);
}

void loop() 
{
  delay(100);
//  if(Wire.available()) {
//    Serial.println("Something");
//  }
}
