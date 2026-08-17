#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

#include "processing/fall.h"
#include "processing/health.h"

//====================================
// DISPLAY DATA
//====================================

struct DisplayData {

    bool testMode=false;

    bool wearing=false;

    bool heartValid=false;

    float bpm=0;

    float spo2=0;
    
    bool spo2Valid=false;

    bool noHeartTimeout;

    health_zone_t zone=
    ZONE_SAFE;

    float batteryPercent=
    0;

    FallState fallState=
    FALL_IDLE;

    int fallCountdown=
    10;
};


//====================================
// CONTROL
//====================================

void display_init();

void display_update(
    const DisplayData&
);

void display_clear();

void display_refresh();

bool display_isActive();

void display_sleep();

void display_wakeup();

#endif