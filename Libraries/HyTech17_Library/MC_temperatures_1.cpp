/*
  MC_temperatures_1.cpp - RMS Inverter CAN message: Temperatures 1
  Created by Nathan Cheek, November 22, 2016.
 */

#include "HyTech17.h"

MC_temperatures_1::MC_temperatures_1() {
    message = {};
}

MC_temperatures_1::MC_temperatures_1(uint8_t buf[8]) {
    load(buf);
}

void MC_temperatures_1::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.module_a_temperature), &buf[0], sizeof(int16_t));
    memcpy(&(message.module_b_temperature), &buf[2], sizeof(int16_t));
    memcpy(&(message.module_c_temperature), &buf[4], sizeof(int16_t));
    memcpy(&(message.gate_driver_board_temperature), &buf[6], sizeof(int16_t));
}

int16_t MC_temperatures_1::get_module_a_temperature() {
  return message.module_a_temperature;
}

int16_t MC_temperatures_1::get_module_b_temperature() {
  return message.module_b_temperature;
}

int16_t MC_temperatures_1::get_module_c_temperature() {
  return message.module_c_temperature;
}

int16_t MC_temperatures_1::get_gate_driver_board_temperature() {
  return message.gate_driver_board_temperature;
}
