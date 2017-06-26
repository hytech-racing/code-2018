/*
  MC_read_write_parameter_response.cpp - RMS Inverter CAN message: Read / Write Parameter Response - response from PM
  Created by Nathan Cheek, November 22, 2016.
 */

#include "HyTech17.h"

MC_read_write_parameter_response::MC_read_write_parameter_response() {
    message = {};
}

MC_read_write_parameter_response::MC_read_write_parameter_response(uint8_t buf[8]) {
    load(buf);
}

void MC_read_write_parameter_response::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.parameter_address), &buf[0], sizeof(uint16_t));
    uint8_t booleanByte = 0;
    memcpy(&booleanByte, &buf[2], sizeof(uint8_t));
    message.write_success = booleanByte > 0;
    memcpy(&(message.reserved1), &buf[3], sizeof(uint8_t));
    memcpy(message.data, &buf[4], sizeof(int32_t));
}

uint16_t MC_read_write_parameter_response::get_parameter_address() {
  return message.parameter_address;
}

bool MC_read_write_parameter_response::get_write_success() {
  return message.write_success;
}

uint8_t* MC_read_write_parameter_response::get_data() {
  return message.data;
}
