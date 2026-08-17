#pragma once
#include <Arduino.h>

//====================================================
// BUZZER MODES
//====================================================
enum BuzzerMode {
    BUZZ_IDLE = 0,
    BUZZ_DANGER,
    BUZZ_FALL,
    BUZZ_FIND
};

//====================================================
// API
//====================================================
void buzzer_init();
void buzzer_update();

void buzzer_setMode(BuzzerMode mode);
void buzzer_stop();

BuzzerMode buzzer_getMode();
bool buzzer_isActive();