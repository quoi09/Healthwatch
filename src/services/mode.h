#include <Arduino.h>
#pragma once

//====================================================
// SYSTEM MODE
//====================================================

enum SystemMode
{
    MODE_REAL = 0,
    MODE_TEST = 1
};

//====================================================
// DEFAULT MODE
//====================================================

#define SYSTEM_MODE MODE_REAL

//====================================================
// API
//====================================================

void mode_init();

SystemMode getSystemMode();

const char* getSystemModeName();

void setSystemMode(SystemMode mode);

void toggleSystemMode();

bool isTestMode();

uint32_t getWarningDelay();

uint32_t getDangerDelay();

uint32_t getSmsCooldown();

uint32_t getCallCooldown();

uint32_t getGpsTimeout();

uint32_t getCallDuration();