#include "button.h"
#include <Arduino.h>
#include "../config.h"

//====================================================
// INTERNAL
//====================================================

static bool currentState = false;
static bool lastReading = false;

static bool shortPressEvent = false;
static bool longPressEvent  = false;

static unsigned long lastDebounce = 0;
static unsigned long pressStart   = 0;

static bool longTriggered = false;

//====================================================
// INIT
//====================================================

void button_init()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

//====================================================
// UPDATE
//====================================================

void button_update()
{
    bool reading = !digitalRead(BUTTON_PIN);

    // debounce
    if(reading != lastReading)
    {
        lastDebounce = millis();
    }

    if(millis() - lastDebounce > 50)
    {
        // BUTTON PRESS
        if(reading && !currentState)
        {
            pressStart = millis();
            longTriggered = false;
        }

        // HOLD 10s
        if(reading && !longTriggered)
        {
            if(millis() - pressStart >= 10000)
            {
                longPressEvent = true;
                longTriggered = true;
            }
        }

        // BUTTON RELEASE
        if(!reading && currentState)
        {
            unsigned long pressTime =
                millis() - pressStart;

            // short press
            if(pressTime < 5000)  // sua lai 10s sau khi test
            {
                shortPressEvent = true;
            }
        }

        currentState = reading;
    }

    lastReading = reading;
}

//====================================================
// STATE
//====================================================

bool button_is_pressed()
{
    return currentState;
}

//====================================================
// SHORT PRESS
//====================================================

bool button_short_press()
{
    if(shortPressEvent)
    {
        shortPressEvent = false;
        return true;
    }

    return false;
}

//====================================================
// LONG PRESS 10s
//====================================================

bool button_long_press()
{
    if(longPressEvent)
    {
        longPressEvent = false;
        return true;
    }

    return false;
}