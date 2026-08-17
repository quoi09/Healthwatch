#include "atgm336h.h"

ATGM336H_Driver::ATGM336H_Driver()
{
    _serial = nullptr;
}

void ATGM336H_Driver::begin(Stream& serialPort)
{
    _serial = &serialPort;
}

void ATGM336H_Driver::update()
{
    if (_serial == nullptr)
        return;

    uint32_t start = millis();

    while (_serial->available())
    {
        char c = _serial->read();

        // parse GPS
        _gps.encode(c);

        // chống block CPU
        if (millis() - start > 5)
            break;
    }
}

bool ATGM336H_Driver::isFixed()
{
    if (_serial == nullptr)
        return false;

    return _gps.location.isValid();
}

float ATGM336H_Driver::latitude()
{
    if(!_gps.location.isValid())
        return 0;

    return _gps.location.lat();
}

float ATGM336H_Driver::longitude()
{
    if(!_gps.location.isValid())
        return 0;

    return _gps.location.lng();
}

float ATGM336H_Driver::altitude()
{
    if(!_gps.altitude.isValid())
        return 0;

    return _gps.altitude.meters();
}

float ATGM336H_Driver::hdop()
{
    if (_gps.hdop.isValid())
        return _gps.hdop.hdop();

    return 99.99;
}

uint32_t ATGM336H_Driver::satellites()
{
    if(!_gps.satellites.isValid())
        return 0;

    return _gps.satellites.value();
}

String ATGM336H_Driver::date()
{
    if (!_gps.date.isValid())
        return "xx/xx/xxxx";

    char buf[20];

    sprintf(
        buf,
        "%02d/%02d/%04d",
        _gps.date.day(),
        _gps.date.month(),
        _gps.date.year()
    );

    return String(buf);
}

String ATGM336H_Driver::time()
{
    if (!_gps.time.isValid())
        return "xx:xx:xx";

    char buf[20];

    sprintf(
        buf,
        "%02d:%02d:%02d",
        _gps.time.hour(),
        _gps.time.minute(),
        _gps.time.second()
    );

    return String(buf);
}