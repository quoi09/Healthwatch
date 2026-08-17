//====================================================
// BUZZER DRIVER
// SIMPLE 1 BEEP / SECOND
//====================================================

#include "buzzer.h"
#include <Arduino.h>

//====================================================
// CONFIG
//====================================================
#define BUZZER_PIN 10
#define BUZZER_ACTIVE HIGH

//====================================================
// GLOBALS
//====================================================
static BuzzerMode currentMode = BUZZ_IDLE;

static bool buzzerState = false;

static uint32_t lastToggle = 0;
static uint32_t modeStart = 0;

//====================================================
// LOW LEVEL
//====================================================
static inline void bz_on()
{
    digitalWrite(BUZZER_PIN, BUZZER_ACTIVE);
    buzzerState = true;
}

static inline void bz_off()
{
    digitalWrite(BUZZER_PIN, !BUZZER_ACTIVE);
    buzzerState = false;
}

//====================================================
// INIT
//====================================================
void buzzer_init()
{
    pinMode(BUZZER_PIN, OUTPUT);

    bz_off();
}

//====================================================
// SET MODE
//====================================================
void buzzer_setMode(BuzzerMode mode)
{
    // FALL luôn ưu tiên cao nhất
    if(currentMode == BUZZ_FALL &&
       mode != BUZZ_FALL)
    {
        return;
    }

    // tránh reset timer liên tục
    if(currentMode == mode)
    {
        return;
    }

    currentMode = mode;

    modeStart = millis();
    lastToggle = millis();

    bz_off();
}

//====================================================
// STOP
//====================================================
void buzzer_stop()
{
    currentMode = BUZZ_IDLE;

    bz_off();
}

//====================================================
// GETTERS
//====================================================
BuzzerMode buzzer_getMode()
{
    return currentMode;
}

bool buzzer_isActive()
{
    return currentMode != BUZZ_IDLE;
}

//====================================================
// COMMON BEEP PATTERN
//
// ON  = 120ms
// OFF = 880ms
//====================================================
static void updateBeep(uint32_t now)
{
    // OFF -> ON
    if(!buzzerState)
    {
        if(now - lastToggle >= 880)
        {
            bz_on();

            lastToggle = now;
        }
    }

    // ON -> OFF
    else
    {
        if(now - lastToggle >= 120)
        {
            bz_off();

            lastToggle = now;
        }
    }
}

//====================================================
// UPDATE
//====================================================
void buzzer_update()
{
    uint32_t now = millis();

    switch(currentMode)
    {
        //------------------------------------------------
        // IDLE
        //------------------------------------------------
        case BUZZ_IDLE:
        {
            bz_off();
            break;
        }

        //------------------------------------------------
        // FALL
        //------------------------------------------------
        case BUZZ_FALL:
        {
            updateBeep(now);
            break;
        }

        //------------------------------------------------
        // DANGER
        //------------------------------------------------
        case BUZZ_DANGER:
        {
            // timeout 30s
            if(now - modeStart >= 30000)
            {
                buzzer_stop();
                return;
            }

            updateBeep(now);

            break;
        }

        
        //------------------------------------------------
        // FIND DEVICE
        //------------------------------------------------
        case BUZZ_FIND:
        {
            // beep nhanh hơn để dễ tìm

            if(!buzzerState)
            {
                if(now - lastToggle >= 200)
                {
                    bz_on();

                    lastToggle = now;
                }
            }
            else
            {
                if(now - lastToggle >= 100)
                {
                    bz_off();

                    lastToggle = now;
                }
            }

            break;
        }


    }
}