/*
  MC_digital_input_status.cpp - RMS Inverter CAN message: Digital Input Status
  Created by Ryan Gallaway, December 1, 2016.
 */

#include "HyTech17.h"

MC_digital_input_status::MC_digital_input_status() {
    message = {};
}

MC_digital_input_status::MC_digital_input_status(uint8_t buf[]) {
  load(buf);
}

void MC_digital_input_status::load(uint8_t buf[]) {
    message = {};
    int index = 0;
    loadByteIntoBooleanStruct(&index, buf, &(message.digital_input_1));
    loadByteIntoBooleanStruct(&index, buf, &(message.digital_input_2));
    loadByteIntoBooleanStruct(&index, buf, &(message.digital_input_3));
    loadByteIntoBooleanStruct(&index, buf, &(message.digital_input_4));
    loadByteIntoBooleanStruct(&index, buf, &(message.digital_input_5));
    loadByteIntoBooleanStruct(&index, buf, &(message.digital_input_6));
    loadByteIntoBooleanStruct(&index, buf, &(message.digital_input_7));
    loadByteIntoBooleanStruct(&index, buf, &(message.digital_input_8));
}

void MC_digital_input_status::loadByteIntoBooleanStruct(int* index, uint8_t buf[], bool* structVariable) {
    uint8_t booleanByte = 0;
    memcpy(&booleanByte, buf + *index, sizeof(char));
    *structVariable = booleanByte > 0;
    *index = *index + sizeof(char);
}

bool MC_digital_input_status::digital_input_1() {
  return message.digital_input_1;
}

bool MC_digital_input_status::digital_input_2() {
  return message.digital_input_2;
}

bool MC_digital_input_status::digital_input_3() {
  return message.digital_input_3;
}

bool MC_digital_input_status::digital_input_4() {
  return message.digital_input_4;
}

bool MC_digital_input_status::digital_input_5() {
  return message.digital_input_5;
}

bool MC_digital_input_status::digital_input_6() {
  return message.digital_input_6;
}

bool MC_digital_input_status::digital_input_7() {
  return message.digital_input_7;
}

bool MC_digital_input_status::digital_input_8() {
  return message.digital_input_8;
}