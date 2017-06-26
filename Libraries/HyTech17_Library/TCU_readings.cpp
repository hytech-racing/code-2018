/*
  TCU_readings.cpp - HyTech Throttle Control Unit CAN message: Readings
  Created by Charith (Karvin) Dassanayake, March 10, 2017.
 */

#include "HyTech17.h"

/*  Blank Constructor for TCU_readings
 *
 * Used to initialize instance of TCU_readings with no data
 */

TCU_readings::TCU_readings() {
    message = {};
}

/* Constructor for TCU_readings using a buffer
 *
 * Used to initialize instance of TCU_readings with data
 * that's in an 8xbyte array (typically msg.buf)
 *
 * Param - Pass in buffer you are trying to initialize data from
 */

TCU_readings::TCU_readings(uint8_t buf[8]) {
  load(buf);
}

/* Constructor for TCU_readings
 *
 * Used to copy data from msg variable in
 * microcontroller code to instance variable
 *
 * Param (uint16_t) - Throttle Value 1
 *     - Raw voltage readings from throttle sensor 1
 * Param (uint16_t) - Throttle Value 2
 *     - Raw voltage readings from throttle sensor 2
 * Param (uint16_t) - Brake Value
 *     - Raw voltage readings from pedal sensor
 * Param (uint16_t) - Temperature reading from temperature sensor
 *     - MAYBE Raw voltage readings from throttle sensor 1
 *     - OR actual temperature estimate
 *     - REVIEW since code not written a this point (Celsius?)
 */

TCU_readings::TCU_readings(uint16_t throttle_value_1, uint16_t throttle_value_2, uint16_t brake_value, uint16_t temperature) {
    set_throttle_value_1(throttle_value_1);
    set_throttle_value_2(throttle_value_2);
    set_brake_value(brake_value);
    set_temperature(temperature);
}

/* Load from buffer & write to variable instance
 *
 * Used to copy data from msg variable in
 * microcontroller code to instance variable
 *
 * Param - Pass in buffer you are trying to read from
 * Example: curTCU_readings.load(msg.buf);
 */

void TCU_readings::load(uint8_t buf[8]) {
    message = {};

    memcpy(&(message.throttle_value_1), &buf[0], sizeof(uint16_t));
    memcpy(&(message.throttle_value_2), &buf[2], sizeof(uint16_t));
    memcpy(&(message.brake_value), &buf[4], sizeof(uint16_t));
    memcpy(&(message.temperature), &buf[6], sizeof(uint16_t));
}

/* Write to buffer
 *
 * Used to copy data from instance of this class
 * to msg variable in microcontroller code
 *
 * Param - Pass in buffer you are trying to modify
 * Example: curTCU_readings.write(msg.buf);
 */

void TCU_readings::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.throttle_value_1), sizeof(uint16_t));
    memcpy(&buf[2], &(message.throttle_value_2), sizeof(uint16_t));
    memcpy(&buf[4], &(message.brake_value), sizeof(uint16_t));
    memcpy(&buf[6], &(message.temperature), sizeof(uint16_t));
}

/* Get functions
 *
 * Used to retrieve values stored in this CAN_message_t
 */

uint16_t TCU_readings::get_throttle_value_1() {
    return message.throttle_value_1;
}

uint16_t TCU_readings::get_throttle_value_2() {
    return message.throttle_value_2;
}

uint16_t TCU_readings::get_throttle_value_avg() {
    return (message.throttle_value_1 + message.throttle_value_2) / 2;
}

uint16_t TCU_readings::get_brake_value() {
    return message.brake_value;
}

uint16_t TCU_readings::get_temperature() {
    return message.temperature;
}

/* Set functions
 *
 * Used to replace values in this CAN_message_t
 *
 * Param (uint16_t) - Variable to replace old data
 */

void TCU_readings::set_throttle_value_1(uint16_t throttle_value_1) {
    message.throttle_value_1 = throttle_value_1;
}

void TCU_readings::set_throttle_value_2(uint16_t throttle_value_2) {
    message.throttle_value_2 = throttle_value_2;
}

void TCU_readings::set_brake_value(uint16_t brake_value) {
    message.brake_value = brake_value;
}

void TCU_readings::set_temperature(uint16_t temperature) {
    message.temperature = temperature;
}
