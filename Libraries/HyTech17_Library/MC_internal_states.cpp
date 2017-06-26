/*
  MC_internal_states.cpp - RMS Inverter CAN message: Internal States
  Created by Nathan Cheek, November 20, 2016.
 */

#include "HyTech17.h"

MC_internal_states::MC_internal_states() {
    message = {};
}

MC_internal_states::MC_internal_states(uint8_t buf[8]) {
  load(buf);
}

void MC_internal_states::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.vsm_state), &buf[0], sizeof(uint16_t));
    memcpy(&(message.inverter_state), &buf[2], sizeof(uint8_t));
    memcpy(&(message.relay_state), &buf[3], sizeof(uint8_t));
    memcpy(&(message.inverter_run_mode_discharge_state), &buf[4], sizeof(uint8_t));
    memcpy(&(message.inverter_command_mode), &buf[5], sizeof(uint8_t));
    memcpy(&(message.inverter_enable), &buf[6], sizeof(uint8_t));
    memcpy(&(message.direction_command), &buf[7], sizeof(uint8_t));
}

uint8_t MC_internal_states::get_vsm_state() {
  return message.vsm_state;
}

uint8_t MC_internal_states::get_inverter_state() {
  return message.inverter_state;
}

bool MC_internal_states::get_relay_active_1() {
  return message.relay_state && 0x01;
}

bool MC_internal_states::get_relay_active_2() {
  return (message.relay_state && 0x02) >> 1;
}

bool MC_internal_states::get_relay_active_3() {
  return (message.relay_state && 0x04) >> 2;
}

bool MC_internal_states::get_relay_active_4() {
  return (message.relay_state && 0x08) >> 3;
}

bool MC_internal_states::get_relay_active_5() {
  return (message.relay_state && 0x10) >> 4;
}

bool MC_internal_states::get_relay_active_6() {
  return (message.relay_state && 0x20) >> 5;
}

bool MC_internal_states::get_inverter_run_mode() {
  return message.inverter_run_mode_discharge_state && 0x01;
}

uint8_t MC_internal_states::get_inverter_active_discharge_state() {
  return (message.inverter_run_mode_discharge_state && 0xE0) >> 5;
}

bool MC_internal_states::get_inverter_command_mode() {
  return message.inverter_command_mode;
}

bool MC_internal_states::get_inverter_enable_state() {
  return message.inverter_enable && 0x01;
}

bool MC_internal_states::get_inverter_enable_lockout() {
  return (message.inverter_enable && 0x80) >> 7;
}

bool MC_internal_states::get_direction_command() {
  return message.direction_command;
}
