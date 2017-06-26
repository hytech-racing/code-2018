/*
  CCU_status.cpp - Cooling Control Unit CAN message
 */

#include "HyTech17.h"

CCU_status::CCU_status() {
  message = {};
}

CCU_status::CCU_status(uint8_t buf[8]) {
  load(buf);
}

void CCU_status::load(uint8_t buf[8]) {
  message = {};
  memcpy(&(message.flow_rate), &buf[0], sizeof(int16_t));
}

uint16_t CCU_status::get_flow_rate() {
  return message.flow_rate;
}

void CCU_status::set_flow_rate(uint16_t flow) {
  message.flow_rate = flow;
}

void CCU_status::write(uint8_t buf[]) {
    memcpy(&buf[0], &(message.flow_rate), sizeof(uint16_t));
}
