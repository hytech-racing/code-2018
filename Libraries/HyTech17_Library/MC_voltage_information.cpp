/*
  MC_voltage_information.cpp - RMS Inverter CAN message: Voltage Information
  Created by Nathan Cheek, November 20, 2016.
 */

#include "HyTech17.h"

MC_voltage_information::MC_voltage_information() {
    message = {};
}

MC_voltage_information::MC_voltage_information(uint8_t buf[8]) {
    load(buf);
}

void MC_voltage_information::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.dc_bus_voltage), &buf[0], sizeof(int16_t));
    memcpy(&(message.output_voltage), &buf[2], sizeof(int16_t));
    memcpy(&(message.phase_ab_voltage), &buf[4], sizeof(int16_t));
    memcpy(&(message.phase_bc_voltage), &buf[6], sizeof(int16_t));
}

int16_t MC_voltage_information::get_dc_bus_voltage() {
  return message.dc_bus_voltage;
}

int16_t MC_voltage_information::get_output_voltage() {
  return message.output_voltage;
}

int16_t MC_voltage_information::get_phase_ab_voltage() {
  return message.phase_ab_voltage;
}

int16_t MC_voltage_information::get_phase_bc_voltage() {
  return message.phase_bc_voltage;
}
