/*
  MC_analog_input_voltages.cpp - RMS Inverter CAN message: Analog Input Voltages
  Created by Ryan Gallaway, December 1, 2016.
 */

#include "HyTech17.h"

MC_analog_input_voltages::MC_analog_input_voltages() {
  message = {};
}

MC_analog_input_voltages::MC_analog_input_voltages(uint8_t buf[8]) {
  load(buf);
}

void MC_analog_input_voltages::load(uint8_t buf[8]) {

  // memcpy(&message, buf, sizeof(CAN_message_mc_analog_input_voltages_t));
    message = {};
    memcpy(&(message.analog_input_1), &buf[0], sizeof(int16_t));
    memcpy(&(message.analog_input_2), &buf[2], sizeof(int16_t));
    memcpy(&(message.analog_input_3), &buf[4], sizeof(int16_t));
    memcpy(&(message.analog_input_4), &buf[6], sizeof(int16_t));
}

int16_t MC_analog_input_voltages::get_analog_input_1() {
  return message.analog_input_1;
}

int16_t MC_analog_input_voltages::get_analog_input_2() {
  return message.analog_input_2;
}

int16_t MC_analog_input_voltages::get_analog_input_3() {
  return message.analog_input_3;
}

int16_t MC_analog_input_voltages::get_analog_input_4() {
  return message.analog_input_4;
}
