#ifndef __ATGM336H_H__
#define __ATGM336H_H__

#include <Arduino.h>
#include <TinyGPS++.h>
#include <Stream.h>

class ATGM336H_Driver
{
private:

    TinyGPSPlus _gps;

    Stream* _serial;

public:

    ATGM336H_Driver();

    void begin(Stream& serialPort);

    void update();

    bool isFixed();

    float latitude();

    float longitude();

    float altitude();

    float hdop();

    uint32_t satellites();

    String date();

    String time();
};

#endif