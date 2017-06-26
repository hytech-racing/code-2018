/**
 * Ryan Gallaway
 * Sep 1 2015
 * If 'r' is input, sends msg on ID 0x01
 * If 's' is input, sends msg on ID 0x02
 * Listens for reply on IDs 0x14 and 0x15
 */
//spies on the CAN interface, printing out everything that it sees

#include <FlexCAN.h>

/*
  Sets the pin Arduino uses for the CAN controller.
  Set to 9 if using a SEEEDStudio (black) shield
  Set to 10 if using a SparkFun (red) shield
*/
FlexCAN CAN(500000);
static CAN_message_t msg;
long msTimer = 0;



void setup() {
  Serial.begin(115200); //initialize serial for PC communication

  CAN.begin();
  delay(1000);
  Serial.println("CAN transceiver initialized");
}

void loop() {
  /*
    This section checks if a message is currently circulating on the bus.
  */
  while (CAN.read(msg)) {
    Serial.print("RCV ");
    Serial.print(msg.id);
    Serial.print(": ");
    for (unsigned int i = 0; i < msg.len; i++) {
      Serial.print(msg.buf[i]);
      Serial.print(" ");
    }
    Serial.println();
  }
    
    // You have to readMsgBuf before you can get the ID of the message

//    if(CAN.getCanId() == 0x15) { //Filters so only certain messages will execute this code
//      Serial.print("Potentiometer value: ");
//      Serial.println(buf[0]);
//      sent = 0;
//    }
//    else if(CAN.getCanId() == 0x14) { //Same thing, different filter
//      Serial.print("Received value: ");
//      Serial.println(buf[0]);
//      sent = 0;
//    }
}
