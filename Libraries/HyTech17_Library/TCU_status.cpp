/*
  TCU_status.cpp - HyTech Throttle Control Unit CAN message: Status
  Created by Nathan Cheek, November 20, 2016.
 */

#include "HyTech17.h"

/*  Blank Constructor for TCU_status
 *
 * Used to initialize instance of TCU_status with no data
 */

TCU_status::TCU_status() {
    message = {};
}

/* Constructor for TCU_status using a buffer
 *
 * Used to initialize instance of TCU_status with data
 * that's in an 8xbyte array (typically msg.buf)
 *
 * Param - Pass in buffer you are trying to initialize data from
 */


TCU_status::TCU_status(uint8_t buf[8]) {
  load(buf);
}

/* Constructor for TCU_status
 *
 * Used to copy data from msg variable in
 * microcontroller code to instance variable
 *
 * Param (bool) - Throttle Implausibility
 *     - Pass in true if implausibility exists
 *     - Pass in false if implausibility does not exist
 * Param (bool) - Throttle Curve configuration
 *     - Pass in true if boost mode
 *     - Pass in false if normal mode
 * Param (bool) - Brake Implausibility exists
 *     - Pass in true if brake implausibility exists
 *     - Pass in false if brake implausibility does not exist
 * Param (bool) - Brake Pedal active/engaged
 *     - Pass in true if brake pedal is considered pressed
 *     - Pass in false if brake pedal is not considerd pressed
 * Param (uint8_t) - ECU State
 */

TCU_status::TCU_status(bool throttle_implausibility, bool throttle_curve, bool brake_implausibility, bool brake_pedal_active, uint8_t state) {
    set_throttle_implausibility(throttle_implausibility);
    set_throttle_curve(throttle_curve);
    set_brake_implausibility(brake_implausibility);
    set_brake_pedal_active(brake_pedal_active);
    set_state(state);
}

/* Load from buffer & write to variable instance
 *
 * Used to copy data from msg variable in
 * microcontroller code to instance variable
 *
 * Param - Pass in buffer you are trying to read from
 * Example: curTCU_status.load(msg.buf);
 */

void TCU_status::load(uint8_t buf[8]) {
    message = {};
    uint8_t booleanByte = 0;

    memcpy(&booleanByte, &buf[0], sizeof(uint8_t));
    message.throttle_implausibility = booleanByte > 0;
    memcpy(&booleanByte, &buf[1], sizeof(uint8_t));
    message.throttle_curve = booleanByte > 0;
    memcpy(&booleanByte, &buf[2], sizeof(uint8_t));
    message.brake_implausibility = booleanByte > 0;
    memcpy(&booleanByte, &buf[3], sizeof(uint8_t));
    message.brake_pedal_active = booleanByte > 0;
    memcpy(&message.state, &buf[4], sizeof(uint8_t));
}

/* Write to buffer
 *
 * Used to copy data from instance of this class
 * to msg variable in microcontroller code
 *
 * Param - Pass in buffer you are trying to modify
 * Example: curTCU_status.write(msg.buf);
 */

void TCU_status::write(uint8_t buf[8]) {
    uint8_t booleanByte;

    booleanByte = 0;
    if (message.throttle_implausibility) {
        booleanByte = 1;
    }
    memcpy(&buf[0], &booleanByte, sizeof(uint8_t));

    booleanByte = 0;
    if (message.throttle_curve) {
        booleanByte = 1;
    }
    memcpy(&buf[1], &booleanByte, sizeof(uint8_t));

    booleanByte = 0;
    if (message.brake_implausibility) {
        booleanByte = 1;
    }
    memcpy(&buf[2], &booleanByte, sizeof(uint8_t));

    booleanByte = 0;
    if (message.brake_pedal_active) {
        booleanByte = 1;
    }
    memcpy(&buf[3], &booleanByte, sizeof(uint8_t));
    memcpy(&buf[4], &message.state, sizeof(uint8_t));
}

/* Get functions
 *
 * Used to retrieve values stored in this CAN_message_t
 */

bool TCU_status::get_throttle_implausibility() {
    return message.throttle_implausibility;
}

bool TCU_status::get_throttle_curve() {
    return message.throttle_curve;
}

bool TCU_status::get_brake_implausibility() {
    return message.brake_implausibility;
}

bool TCU_status::get_brake_pedal_active() {
    return message.brake_pedal_active;
}

uint8_t TCU_status::get_state() {
    return message.state;
}

/* Set functions
 *
 * Used to replace values in this CAN_message_t
 *
 * Param (bool) - Variable to replace old data
 */

void TCU_status::set_throttle_implausibility(bool throttle_implausibility) {
    message.throttle_implausibility = throttle_implausibility;
}

void TCU_status::set_throttle_curve(bool throttle_curve) {
    message.throttle_curve = throttle_curve;
}

void TCU_status::set_brake_implausibility(bool brake_implausibility) {
    message.brake_implausibility = brake_implausibility;
}

void TCU_status::set_brake_pedal_active(bool brake_pedal_active) {
    message.brake_pedal_active = brake_pedal_active;
}

void TCU_status::set_state(uint8_t new_state) {
    message.state = new_state;
}
