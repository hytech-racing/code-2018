/*
 * Charge Board Status
 * Very simple
 * Ryan Gallaway
 */
#include "HyTech17.h"

Charge_status::Charge_status() {
    message = {};
}

Charge_status::Charge_status(uint8_t buf[8]) {
    load(buf);
}

void Charge_status::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.charge_command), &buf[0], sizeof(uint8_t));
}

void Charge_status::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.charge_command), sizeof(uint8_t));
}

uint8_t Charge_status::getChargeCommand() {
    return message.charge_command;
}

void Charge_status::setChargeCommand(uint8_t cmd) {
    message.charge_command = cmd;
}
