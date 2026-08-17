#include "mode.h"
#include <Preferences.h>

//====================================================
// PREFERENCES
//====================================================

static Preferences prefs;

//====================================================
// RUNTIME MODE
//====================================================

static SystemMode systemModeRuntime =
    SYSTEM_MODE;

//====================================================
// REAL MODE PROFILE
//====================================================

static const uint32_t REAL_SMS_COOLDOWN_MS  = 00000UL;
static const uint32_t REAL_CALL_COOLDOWN_MS = 00000UL;
static const uint32_t REAL_GPS_TIMEOUT_MS   = 0000UL;
static const uint32_t REAL_CALL_DURATION_MS = 5000UL;

//====================================================
// TEST MODE PROFILE
//====================================================

static const uint32_t TEST_SMS_COOLDOWN_MS  = 0000UL;
static const uint32_t TEST_CALL_COOLDOWN_MS = 0000UL;
static const uint32_t TEST_GPS_TIMEOUT_MS   = 0UL;
static const uint32_t TEST_CALL_DURATION_MS = 3000UL;

//====================================================
// INIT
//====================================================

void mode_init()
{
    prefs.begin("system", false);

    if(prefs.isKey("mode"))
    {
        systemModeRuntime =
            (SystemMode)prefs.getInt(
                "mode",
                SYSTEM_MODE
            );
    }
}

//====================================================
// GET MODE
//====================================================

SystemMode getSystemMode()
{
    return systemModeRuntime;
}

bool isTestMode()
{
    return (systemModeRuntime == MODE_TEST);
}

//====================================================
// GET MODE NAME
//====================================================

const char* getSystemModeName()
{
    return
        (systemModeRuntime == MODE_REAL)
        ? "REAL"
        : "TEST";
}

//====================================================
// SET MODE
//====================================================

void setSystemMode(SystemMode mode)
{
    systemModeRuntime = mode;

    prefs.putInt("mode", mode);
}

//====================================================
// TOGGLE MODE
//====================================================

void toggleSystemMode()
{
    if(systemModeRuntime == MODE_REAL)
    {
        setSystemMode(MODE_TEST);
    }
    else
    {
        setSystemMode(MODE_REAL);
    }
}

//====================================================
// ALERT DELAY PROFILE
//====================================================

uint32_t getWarningDelay()
{
    if(isTestMode())
    {
        return 5000UL;
    }

    return 30000UL;
}

uint32_t getDangerDelay()
{
    if(isTestMode())
    {
        return 3000UL;
    }

    return 15000UL;
}

//====================================================
// ALERT PROFILE GETTERS
//====================================================

uint32_t getSmsCooldown()
{
    if(isTestMode())
    {
        return TEST_SMS_COOLDOWN_MS;
    }

    return REAL_SMS_COOLDOWN_MS;
}

uint32_t getCallCooldown()
{
    if(isTestMode())
    {
        return TEST_CALL_COOLDOWN_MS;
    }

    return REAL_CALL_COOLDOWN_MS;
}

uint32_t getGpsTimeout()
{
    if(isTestMode())
    {
        return TEST_GPS_TIMEOUT_MS;
    }

    return REAL_GPS_TIMEOUT_MS;
}

uint32_t getCallDuration()
{
    if(isTestMode())
    {
        return TEST_CALL_DURATION_MS;
    }

    return REAL_CALL_DURATION_MS;
}