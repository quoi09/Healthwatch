//====================================================
// power_mgr.h
//====================================================
#pragma once

#include <Arduino.h>

//====================================================
// INIT
//====================================================
void power_init();

//====================================================
// MAIN UPDATE
//====================================================
void power_update();

//====================================================
// EVENT NOTIFY
//====================================================
void power_notifyButton();
void power_notifyDisplay();
void power_notifyMotion();
void power_notifyAlert();

//====================================================
// SAFE MODE
//====================================================
void power_enterSafeMode();

//====================================================
// GPS
//====================================================
void power_requestGPS();
void power_releaseGPS();

bool power_gpsReady();
bool power_gpsActive();

//====================================================
// GSM
//====================================================
void power_requestGSM();
void power_releaseGSM();

bool power_gsmReady();
bool power_gsmActive();

//====================================================
// DISPLAY
//====================================================
bool power_displayActive();

//====================================================
// LOCK
//====================================================
void power_lock();
void power_unlock();

bool power_isLocked();

//====================================================
// GPS EMERGENCY
//====================================================
void power_enableEmergencyGPS();
void power_disableEmergencyGPS();

//====================================================
// SENSOR POWER
//====================================================
void power_sleepHeartSensor();   // MAX30102 OFF
void power_wakeupHeartSensor();  // MAX30102 ON

void power_motionMode();         // MPU WOM
void power_normalMode();         // MPU FULL

//====================================================
// SYSTEM SLEEP
//====================================================
bool power_canLightSleep();

void power_enterLightSleep();

//====================================================
// GPS STATUS
//====================================================
bool power_hasFirstFix();