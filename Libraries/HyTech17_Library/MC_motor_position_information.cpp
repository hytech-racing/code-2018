/*
  MC_motor_position_information.cpp - RMS Inverter CAN message: Motor Position Information
  Created by Nathan Cheek, November 22, 2016.
 */

#include "HyTech17.h"

MC_motor_position_information::MC_motor_position_information() {
    message = {};
}

MC_motor_position_information::MC_motor_position_information(uint8_t buf[8]) {
  load(buf);
}

void MC_motor_position_information::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.motor_angle), &buf[0], sizeof(int16_t));
    memcpy(&(message.motor_speed), &buf[2], sizeof(int16_t));
    memcpy(&(message.electrical_output_frequency), &buf[4], sizeof(int16_t));
    memcpy(&(message.delta_resolver_filtered), &buf[6], sizeof(int16_t));
}

int16_t MC_motor_position_information::get_motor_angle() {
  return message.motor_angle;
}

int16_t MC_motor_position_information::get_motor_speed() {
  return message.motor_speed;
}

int16_t MC_motor_position_information::get_electrical_output_frequency() {
  return message.electrical_output_frequency;
}

int16_t MC_motor_position_information::get_delta_resolver_filtered() {
  return message.delta_resolver_filtered;
}
