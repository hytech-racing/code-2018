/*
 * MC_temperatures_2.cpp - CAN message parser: RMS Motor Controller temperatures 2 message
 * Created by Nathan Cheek, November 22, 2016.
 */

#include "HyTech17.h"

MC_temperatures_2::MC_temperatures_2() {
    message = {};
}

MC_temperatures_2::MC_temperatures_2(uint8_t buf[8]) {
    load(buf);
}

void MC_temperatures_2::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.control_board_temperature), &buf[0], sizeof(int16_t));
    memcpy(&(message.rtd_1_temperature), &buf[2], sizeof(int16_t));
    memcpy(&(message.rtd_2_temperature), &buf[4], sizeof(int16_t));
    memcpy(&(message.rtd_3_temperature), &buf[6], sizeof(int16_t));
}

void MC_temperatures_2::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.control_board_temperature), sizeof(int16_t));
    memcpy(&buf[2], &(message.rtd_1_temperature), sizeof(int16_t));
    memcpy(&buf[4], &(message.rtd_2_temperature), sizeof(int16_t));
    memcpy(&buf[6], &(message.rtd_3_temperature), sizeof(int16_t));
}

int16_t MC_temperatures_2::get_control_board_temperature() {
    return message.control_board_temperature;
}

int16_t MC_temperatures_2::get_rtd_1_temperature() {
    return message.rtd_1_temperature;
}

int16_t MC_temperatures_2::get_rtd_2_temperature() {
    return message.rtd_2_temperature;
}

int16_t MC_temperatures_2::get_rtd_3_temperature() {
    return message.rtd_3_temperature;
}
