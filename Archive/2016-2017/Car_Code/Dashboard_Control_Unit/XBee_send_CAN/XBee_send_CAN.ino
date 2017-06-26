// Sends CAN Data

#include <FlexCAN.h>
#include <Metro.h>
#include "HyTech17.h"

#define XB Serial2            //To communicate with XBee
//#define SDCard Serial3      //Print to SD card

unsigned long baud = 115200;
const int led_pin = 13;
byte buff[80];

FlexCAN CAN(500000);            // Establish CAN baud rate
CAN_message_t msg;              // Variable for CAN message
Metro timer_light = Metro(3);

//File CAN_Data;
//const int chipSelect = BUILTIN_SDCARD;

void setup() {
    Serial.begin(baud);           //initialize serial for PC communication
    XB.begin(baud);               //Begin XBee communication
    CAN.begin();              //Begin CAN communication
    delay(1000);
    pinMode(led_pin, OUTPUT); 
    for (unsigned int i = 0; i < 3; i++) {
      digitalWrite(13,HIGH);
      delay(100);
      digitalWrite(13,LOW);
      delay(100);
  }
    Serial.println(ID_MC_TEMPERATURES_1);             // Print msg ID's for reference if needed
    Serial.println(ID_MC_TEMPERATURES_3);
    Serial.println(ID_MC_MOTOR_POSITION_INFORMATION);
    Serial.println(ID_MC_CURRENT_INFORMATION);
    Serial.println(ID_MC_VOLTAGE_INFORMATION);
    Serial.println(ID_MC_INTERNAL_STATES);
    Serial.println(ID_MC_FAULT_CODES); 
    Serial.println(ID_MC_TORQUE_TIMER_INFORMATION);
    Serial.println("--------------------------------");
    
    Serial.println("Communication with CAN bus initialized.");

    /*SDCard.begin(baud);
    Serial.println("Communication with SD card initialized.");
    
    msg.id = 0x1;
    msg.len = 8;

    CAN_Data = SD.open("CAN Bus Data", FILE_WRITE);*/
}

//byte buffer[80]; // Does nothing?
int numSent,k = 0;

void loop() {

  int wr;
  while(CAN.available()) { // Receive a message on CAN

          //digitalWrite(led_pin, HIGH);
    delay(8);
          //digitalWrite(led_pin, LOW);
          //delay(8);
          
    CAN.read(msg);
    //Serial.println(msg.id, HEX);

      wr = XB.availableForWrite();     // check if the hardware serial port is ready to transmit
      if (wr > 1) {
      //If a message is received, check if that msg id is actually something we care about
      //If it is, copy the message to buff and send buff over XBee
      //Delay is necessary to let the code "think"

      if ((msg.id == ID_MC_TEMPERATURES_1) || 
         (msg.id == ID_MC_TEMPERATURES_3) || 
         (msg.id == ID_MC_MOTOR_POSITION_INFORMATION) ||
         (msg.id == ID_MC_CURRENT_INFORMATION) ||
         (msg.id == ID_MC_VOLTAGE_INFORMATION) || 
         (msg.id == ID_MC_INTERNAL_STATES) ||
         (msg.id == ID_MC_FAULT_CODES) || 
         (msg.id == ID_MC_TORQUE_TIMER_INFORMATION)) {
          Serial.println(msg.id, HEX);
          Serial.println("-------------------------------------------------------------------------------------------------------------");
          memcpy(buff,&msg,sizeof(msg));
//          for (int i = 0; i < sizeof(buff); i++) Serial.print(buff[i]);
//          Serial.println();
          numSent = XB.write(buff, sizeof(msg));
          digitalWrite(13,HIGH);
          delay(10);
          digitalWrite(13,LOW);
          delay(10);
          //XB.write(5);
          //Serial.println(numSent);

          //if (CAN_Data.available > 0) {
          //  SDCard.println(buff);
          }
      
 
    }
  }

  // check if the USB virtual serial wants a new baud rate
  if (Serial.baud() != baud) {
    Serial.print("Current Baud:    ----------------------------------------------------------------------------------------- ");
    Serial.println(baud);
    baud = Serial.baud();
    if (baud == 57600) {
      Serial.println("RESET BAUD -----------------------------------------------------------------------------------------");
      // This ugly hack is necessary for talking
      // to the arduino bootloader, which actually
      // communicates at 58824 baud (+2.1% error).
      // Teensyduino will configure the UART for
      // the closest baud rate, which is 57143
      // baud (-0.8% error).  Serial communication
      // can tolerate about 2.5% error, so the
      // combined error is too large.  Simply
      // setting the baud rate to the same as
      // arduino's actual baud rate works.
      XB.begin(58824);
    } else {
      XB.begin(baud);
    }
  }

  timer_light.reset();

}
