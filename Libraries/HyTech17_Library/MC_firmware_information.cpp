/*
  MC_firmware_information.cpp - RMS Inverter CAN message: Firmware Information
  Created by Nathan Cheek, November 23, 2016.
 */

#include "HyTech17.h"

MC_firmware_information::MC_firmware_information() {
    message = {};
}

MC_firmware_information::MC_firmware_information(uint8_t buf[8]) {
  load(buf);
}

void MC_firmware_information::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.eeprom_version_project_code), &buf[0], sizeof(uint16_t));
    memcpy(&(message.software_version), &buf[2], sizeof(uint16_t));
    memcpy(&(message.date_code_mmdd), &buf[4], sizeof(uint16_t));
    memcpy(&(message.date_code_yyyy), &buf[6], sizeof(uint16_t));
}

uint16_t MC_firmware_information::get_eeprom_version_project_code() {
  return message.eeprom_version_project_code;
}

uint16_t MC_firmware_information::get_software_version() {
  return message.software_version;
}

uint16_t MC_firmware_information::get_date_code_mmdd() {
  return message.date_code_mmdd;
}

uint16_t MC_firmware_information::get_date_code_yyyy() {
  return message.date_code_yyyy;
}
