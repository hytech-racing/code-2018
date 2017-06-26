/*
  DCU_status.cpp - HyTech Dashboard Control Unit CAN message: Status
  Created by Karvin Dassanayake, February 12, 2017.
 */

#include "HyTech17.h"

DCU_status::DCU_status() {
    message = {};
}

DCU_status::DCU_status(uint8_t buf[8]) {
  load(buf);
}

DCU_status::DCU_status(uint8_t btn_press_id, uint8_t light_active_1, uint8_t light_active_2, uint8_t rtds_state) {
    set_btn_press_id(btn_press_id);
    set_light_active_1(light_active_1);
    set_light_active_2(light_active_2);
    set_rtds_state(rtds_state);
}

void DCU_status::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.btn_press_id), &buf[0], sizeof(uint8_t));
    memcpy(&(message.light_active_1), &buf[1], sizeof(uint8_t));
    memcpy(&(message.light_active_2), &buf[2], sizeof(uint8_t));
    memcpy(&(message.rtds_state), &buf[3], sizeof(uint8_t));
}

void DCU_status::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.btn_press_id), sizeof(uint8_t));
    memcpy(&buf[1], &(message.light_active_1), sizeof(uint8_t));
    memcpy(&buf[2], &(message.light_active_2), sizeof(uint8_t));
    memcpy(&buf[3], &(message.rtds_state), sizeof(uint8_t));
}

uint8_t DCU_status::get_btn_press_id() {
  return message.btn_press_id;
}

uint8_t DCU_status::get_light_active_1() {
  return message.light_active_1;
}

uint8_t DCU_status::get_light_active_2() {
  return message.light_active_2;
}

uint8_t DCU_status::get_rtds_state() {
  return message.rtds_state;
}

void DCU_status::set_btn_press_id(uint8_t btn_press_id) {
  message.btn_press_id = btn_press_id;
}

void DCU_status::set_light_active_1(uint8_t light_active_1) {
  message.light_active_1 = light_active_1;
}

void DCU_status::set_light_active_2(uint8_t light_active_2) {
  message.light_active_2 = light_active_2;
}

void DCU_status::set_rtds_state(uint8_t rtds_state) {
  message.rtds_state = rtds_state;
}
