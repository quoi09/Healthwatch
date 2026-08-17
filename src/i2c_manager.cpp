#include "i2c_manager.h"

#include <Wire.h>

#include "config.h"

std::vector<uint8_t> foundDevices;

void i2c_init()
{
    Wire.begin(
                SDA_PIN,
                SCL_PIN
               );

    Wire.setClock(I2C_FREQ);

    DEBUG_PRINTLN("I2C init done");
}



std::vector<uint8_t> i2c_scan(bool showOnOled)
{
    foundDevices.clear();

    DEBUG_PRINTLN("Scanning I2C...");

    for(uint8_t addr=1; addr<127; addr++)
    {
        Wire.beginTransmission(addr);

        uint8_t err=Wire.endTransmission();

        if(err==0)
        {
            foundDevices.push_back(addr);

            DEBUG_PRINT("I2C device found at 0x");

            if(addr<16)
                DEBUG_PRINT("0");

            DEBUG_PRINTLN(addr,HEX);

            delay(2);
        }
    }


    DEBUG_PRINT("Total I2C devices: ");

    DEBUG_PRINTLN(foundDevices.size());


    if(foundDevices.empty())
    {
        DEBUG_PRINTLN("No I2C device found");
    }

    return foundDevices;
}