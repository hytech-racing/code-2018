/*
  MC_command_message.cpp - HyTech Throttle Control Unit CAN message: Status
  Created by Nathan Cheek, November 20, 2016.
 */

#include "HyTech17.h"

MC_command_message::MC_command_message() {
  message = {};
}

MC_command_message::MC_command_message(uint8_t buf[8]) {
  load(buf);
}

MC_command_message::MC_command_message(int16_t torque_command, int16_t angular_velocity, bool direction, bool inverter_enable, bool discharge_enable, int16_t commanded_torque_limit) {
  message = {};
  set_torque_command(torque_command);
  set_angular_velocity(angular_velocity);
  set_direction(direction);
  set_inverter_enable(inverter_enable);
  set_discharge_enable(discharge_enable);
  set_commanded_torque_limit(commanded_torque_limit);
}


void MC_command_message::load(uint8_t buf[]) {
  memcpy(&(message.torque_command), &buf[0], sizeof(uint16_t));
  memcpy(&(message.angular_velocity), &buf[2], sizeof(uint16_t));
  unsigned char booleanByte1;
  memcpy(&booleanByte1, &buf[4], sizeof(char));
  message.direction = (booleanByte1 & 0b10000000) > 0;
  memcpy(&(message.inverter_enable_discharge_enable), &buf[5], sizeof(uint8_t));
  memcpy(&(message.commanded_torque_limit), &buf[6], sizeof(uint16_t));
}

void MC_command_message::write(uint8_t buf[8]) {
  memcpy(&buf[0], &(message.torque_command), sizeof(uint16_t));
  memcpy(&buf[2], &(message.angular_velocity), sizeof(uint16_t));
  unsigned char booleanByte1 = 0;
  if (message.direction) {
      booleanByte1 = 1;
  }
  memcpy(&buf[4], &booleanByte1, sizeof(char));
  memcpy(&buf[5], &(message.inverter_enable_discharge_enable), sizeof(uint8_t));
  memcpy(&buf[6], &(message.commanded_torque_limit), sizeof(uint16_t));
}

int16_t MC_command_message::get_torque_command() {
  return message.torque_command;
}

int16_t MC_command_message::get_angular_velocity() {
  return message.angular_velocity;
}

bool MC_command_message::get_direction() {
  return message.direction;
}

bool MC_command_message::get_inverter_enable() {
  return message.inverter_enable_discharge_enable & 0x1;
}

bool MC_command_message::get_discharge_enable() {
  return (message.inverter_enable_discharge_enable & 0x2) >> 1;
}

int16_t MC_command_message::get_commanded_torque_limit() {
  return message.commanded_torque_limit;
}

void MC_command_message::set_torque_command(int16_t torque_command) {
  message.torque_command = torque_command;
}

void MC_command_message::set_angular_velocity(int16_t angular_velocity) {
  message.angular_velocity = angular_velocity;
}

void MC_command_message::set_direction(bool direction) {
  message.direction = direction & 0x1;
}

void MC_command_message::set_inverter_enable(bool inverter_enable) {
  message.inverter_enable_discharge_enable = (message.inverter_enable_discharge_enable & 0xFE) | (inverter_enable & 0x1);
}

void MC_command_message::set_discharge_enable(bool discharge_enable) {
  message.inverter_enable_discharge_enable = (message.inverter_enable_discharge_enable & 0xFD) | ((discharge_enable & 0x1) << 1);
}

void MC_command_message::set_commanded_torque_limit(int16_t commanded_torque_limit) {
  message.commanded_torque_limit = commanded_torque_limit;
}
