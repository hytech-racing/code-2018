/*
  BMS_voltages.cpp - BMS voltage messaging handler
  Created by Shrivathsav Seshan, January 10, 2017.
 */

#include "HyTech17.h"

BMS_voltages::BMS_voltages() {
    bmsVoltageMessage = {};
}

/*
 * [b0 b1 b2 b3 b4 b5 b6 b7]
 * [avg avg low low high high - -]
 */
BMS_voltages::BMS_voltages(uint8_t buf[]) {
    load(buf);
}

BMS_voltages::BMS_voltages(uint16_t avg, uint16_t low, uint16_t high, uint16_t total) {
    bmsVoltageMessage = {};
    bmsVoltageMessage.avgVoltage = avg;
    bmsVoltageMessage.lowVoltage = low;
    bmsVoltageMessage.highVoltage = high;
    bmsVoltageMessage.totalVoltage = total;
}

/*
 * Populate this object using the data stored in the specified byte array.
 */
void BMS_voltages::load(uint8_t buf[]) {
    memcpy(&(bmsVoltageMessage.avgVoltage), &buf[0], sizeof(uint16_t));
    memcpy(&(bmsVoltageMessage.lowVoltage), &buf[2], sizeof(uint16_t));
    memcpy(&(bmsVoltageMessage.highVoltage), &buf[4], sizeof(uint16_t));
    memcpy(&(bmsVoltageMessage.totalVoltage), &buf[6], sizeof(uint16_t));
}

/*
 * Populates the specified byte array using the data stored in this object.
 */
void BMS_voltages::write(uint8_t buf[]) {
    memcpy(&buf[0], &bmsVoltageMessage.avgVoltage, sizeof(uint16_t));
    memcpy(&buf[2], &bmsVoltageMessage.lowVoltage, sizeof(uint16_t));
    memcpy(&buf[4], &bmsVoltageMessage.highVoltage, sizeof(uint16_t));
    memcpy(&buf[6], &bmsVoltageMessage.totalVoltage, sizeof(uint16_t));
}

uint16_t BMS_voltages::getAverage() {
    return bmsVoltageMessage.avgVoltage;
}

uint16_t BMS_voltages::getLow() {
    return bmsVoltageMessage.lowVoltage;
}

uint16_t BMS_voltages::getHigh() {
    return bmsVoltageMessage.highVoltage;
}

uint16_t BMS_voltages::getTotal() {
    return bmsVoltageMessage.totalVoltage;
}

void BMS_voltages::setAverage(uint16_t avg) {
    bmsVoltageMessage.avgVoltage = avg;
}

void BMS_voltages::setLow(uint16_t low) {
    bmsVoltageMessage.lowVoltage = low;
}

void BMS_voltages::setHigh(uint16_t high) {
    bmsVoltageMessage.highVoltage = high;
}

void BMS_voltages::setTotal(uint16_t total) {
    bmsVoltageMessage.totalVoltage = total;
}
