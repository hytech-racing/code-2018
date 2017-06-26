#include <FlexCAN.h>

FlexCAN CAN(500000);

CAN_message_t msg;


void setup() {
  
  Serial.begin(115200); //initialize serial for PC communication

  CAN.begin();
  delay(1000);
  Serial.println("CAN transceiver initialized");

  msg.id = 0x1;
  msg.len = 2;
  msg.buf[1] = 0x1;

}

void loop() {
  
  msg.buf[0] = millis();
  CAN.write(msg);
  delay(2000);
  
}
