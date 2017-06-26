/**
 * Nathan Cheek
 * 2016-10-21
 * Read data from Pedal Box test board and turn on and off solid state relays
 */

#include <FlexCAN.h>
#include <Metro.h>

FlexCAN CAN(500000);
static CAN_message_t msg;
long msTimer = 0;

boolean brakeLight = false;

Metro latchTimer = Metro(1000);
boolean shutdownCircuitInitialized = false;
boolean shutdownCircuitInitializing = false;

void setup() {
  Serial.begin(115200); //initialize serial for PC communication

  CAN.begin();
  delay(1000);
  Serial.println("CAN transceiver initialized");

  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(13, OUTPUT);
  
}

void loop() {
  /*
   * Handle incoming CAN messages
   */
  while (CAN.read(msg)) {
    Serial.print(msg.id);
    Serial.print(": ");
    
    for (unsigned int i = 0; i < msg.len; i += 2) {
      int value = msg.buf[i] + (msg.buf[i+1] << 8);
      float voltage = (float)value / 155.1515;
      if (i == 0) {
        if (voltage >= 2) {
          brakeLight = true;
        }
        else {
          brakeLight = false;
        }
      }
      if (i == 6 && voltage >= 3 && !shutdownCircuitInitialized && !shutdownCircuitInitializing) {
        shutdownCircuitInitializing = true;
        latchTimer.reset();
        digitalWrite(10, HIGH);
        digitalWrite(11, HIGH);
      }
      Serial.print(voltage);
      Serial.print("V ");
    }
    Serial.print(millis());
    Serial.println();

    if (brakeLight) {
      digitalWrite(13, HIGH);
    } else {
      digitalWrite(13, LOW);
    }

    if (shutdownCircuitInitializing && latchTimer.check()) {
      shutdownCircuitInitializing = false;
      shutdownCircuitInitialized = true;
      digitalWrite(10, LOW);
    }
  }
}
