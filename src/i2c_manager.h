// src/i2c_manager.h
#ifndef I2C_MANAGER_H
#define I2C_MANAGER_H

#include <Arduino.h>
#include <vector>

// Init I2C bus
void i2c_init();

// Scan all I2C devices
// showOnOled = true -> show results on OLED
std::vector<uint8_t> i2c_scan(bool showOnOled = true);

#endif