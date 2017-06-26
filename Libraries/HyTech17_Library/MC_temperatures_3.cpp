/*
  MC_temperatures_3.cpp - RMS Inverter CAN message: Temperatures 2
  Created by Nathan Cheek, November 22, 2016.
 */

#include "HyTech17.h"

MC_temperatures_3::MC_temperatures_3() {
    message = {};
}

MC_temperatures_3::MC_temperatures_3(uint8_t buf[8]) {
    load(buf);
}

void MC_temperatures_3::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.rtd_4_temperature), &buf[0], sizeof(int16_t));
    memcpy(&(message.rtd_5_temperature), &buf[2], sizeof(int16_t));
    memcpy(&(message.motor_temperature), &buf[4], sizeof(int16_t));
    memcpy(&(message.torque_shudder), &buf[6], sizeof(int16_t));
}

int16_t MC_temperatures_3::get_rtd_4_temperature() {
  return message.rtd_4_temperature;
}

int16_t MC_temperatures_3::get_rtd_5_temperature() {
  return message.rtd_5_temperature;
}

int16_t MC_temperatures_3::get_motor_temperature() {
  return message.motor_temperature;
}

int16_t MC_temperatures_3::get_torque_shudder() {
  return message.torque_shudder;
}
