/*
  BMS_temperatures.cpp - BMS temperature messaging handler
  Created by Shrivathsav Seshan, February 7, 2017.
 */

#include "HyTech17.h"

BMS_temperatures::BMS_temperatures() {
    bmsTemperatureMessage = {};
}

BMS_temperatures::BMS_temperatures(uint8_t buf[]) {
    load(buf);
}

BMS_temperatures::BMS_temperatures(uint16_t avg, uint16_t low, uint16_t high) {
    setAvgTemp(avg);
    setLowTemp(low);
    setHighTemp(high);
}

void BMS_temperatures::load(uint8_t buf[]) {
    bmsTemperatureMessage = {};
    memcpy(&(bmsTemperatureMessage.avgTemp), &buf[0], sizeof(uint16_t));
    memcpy(&(bmsTemperatureMessage.lowTemp), &buf[2], sizeof(uint16_t));
    memcpy(&(bmsTemperatureMessage.highTemp), &buf[4], sizeof(uint16_t));
}

void BMS_temperatures::write(uint8_t buf[]) {
    memcpy(&buf[0], &(bmsTemperatureMessage.avgTemp), sizeof(uint16_t));
    memcpy(&buf[2], &(bmsTemperatureMessage.lowTemp), sizeof(uint16_t));
    memcpy(&buf[4], &(bmsTemperatureMessage.highTemp), sizeof(uint16_t));
}

uint16_t BMS_temperatures::getAvgTemp() {
    return bmsTemperatureMessage.avgTemp;
}

uint16_t BMS_temperatures::getLowTemp() {
    return bmsTemperatureMessage.lowTemp;
}

uint16_t BMS_temperatures::getHighTemp() {
    return bmsTemperatureMessage.highTemp;
}

void BMS_temperatures::setAvgTemp(uint16_t avg) {
    bmsTemperatureMessage.avgTemp = avg;
}

void BMS_temperatures::setLowTemp(uint16_t low) {
    bmsTemperatureMessage.lowTemp = low;
}

void BMS_temperatures::setHighTemp(uint16_t high) {
    bmsTemperatureMessage.highTemp = high;
}
