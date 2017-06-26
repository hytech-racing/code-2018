#include <FlexCAN.h>
#include <Metro.h>

FlexCAN CAN(500000);

CAN_message_t msg;

Metro canTimer = Metro(1);
Metro lightTimer = Metro(250);


void setup() {
  
  Serial.begin(115200); //initialize serial for PC communication

  pinMode(13, OUTPUT);

  CAN.begin();
  delay(1000);
  Serial.println("CAN transceiver initialized");

  msg.id = 0x1;
  msg.len = 8;

}

void loop() {
  if (lightTimer.check()) {
    digitalWrite(13, LOW);
  }

  /*
   * Send raw values of throttle and BSPD inputs over CAN Bus
   */
  if (canTimer.check()) {
    digitalWrite(13, HIGH);
    lightTimer.reset();
    msg.buf[0] = analogRead(0) & 0xFF;
    msg.buf[1] = analogRead(0) >> 8;
    msg.buf[2] = analogRead(1) & 0xFF;
    msg.buf[3] = analogRead(1) >> 8;
    msg.buf[4] = analogRead(2) & 0xFF;
    msg.buf[5] = analogRead(2) >> 8;
    msg.buf[6] = analogRead(3) & 0xFF;
    msg.buf[7] = analogRead(3) >> 8;
    CAN.write(msg);
  }
}
