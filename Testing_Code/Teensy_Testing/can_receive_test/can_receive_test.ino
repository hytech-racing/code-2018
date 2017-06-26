#include <FlexCAN.h>
#include <Metro.h>

FlexCAN CAN(500000);
CAN_message_t msg;
Metro timer_light = Metro(3);

void setup() {
  Serial.begin(115200); //initialize serial for PC communication
  CAN.begin();
  delay(3000);
  pinMode(13, OUTPUT);
  for (unsigned int i = 0; i < 3; i++) {
    digitalWrite(13,HIGH);
    delay(250);
    digitalWrite(13,LOW);
    delay(250);
  }
  Serial.println("CAN transceiver initialized");
  Serial.println("CAN TEST RECEIVER");
}

void loop() {
  while (CAN.read(msg)) { // Receive a message on CAN
    Serial.print("Received 0x");
    Serial.print(msg.id, HEX);
    Serial.print(": ");
    for (unsigned int i = 0; i < msg.len; i++) {
      Serial.print(msg.buf[i]);
      Serial.print(" ");
    }
    Serial.println();
    digitalWrite(13, HIGH);
    timer_light.reset();
  }
  if (timer_light.check()) { // Turn off LED
    digitalWrite(13, LOW);
  }
}
