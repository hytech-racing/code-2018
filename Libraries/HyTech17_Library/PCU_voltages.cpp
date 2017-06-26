/*
  PCU_voltages.cpp - HyTech Power Control Unit CAN message: Voltages
  Created by Karvin Dassanayake, March 2, 2017
 */

#include "HyTech17.h"

PCU_voltages::PCU_voltages() {
    message = {};
}

PCU_voltages::PCU_voltages(uint8_t buf[8]) {
  load(buf);
}

PCU_voltages::PCU_voltages(uint16_t GLV_battery_voltage, uint16_t shutdown_circuit_voltage) {
    set_GLV_battery_voltage(GLV_battery_voltage);
    set_shutdown_circuit_voltage(shutdown_circuit_voltage);
}

void PCU_voltages::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.GLV_battery_voltage), &buf[0], sizeof(uint16_t));
    memcpy(&(message.shutdown_circuit_voltage), &buf[2], sizeof(uint16_t));
}

void PCU_voltages::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.GLV_battery_voltage), sizeof(uint16_t));
    memcpy(&buf[2], &(message.shutdown_circuit_voltage), sizeof(uint16_t));
}

uint16_t PCU_voltages::get_GLV_battery_voltage() {
  return message.GLV_battery_voltage;
}

uint16_t PCU_voltages::get_shutdown_circuit_voltage() {
  return message.shutdown_circuit_voltage;
}

void PCU_voltages::set_GLV_battery_voltage(uint16_t GLV_battery_voltage) {
  message.GLV_battery_voltage = GLV_battery_voltage;
}

void PCU_voltages::set_shutdown_circuit_voltage(uint16_t shutdown_circuit_voltage) {
  message.shutdown_circuit_voltage = shutdown_circuit_voltage;
}
