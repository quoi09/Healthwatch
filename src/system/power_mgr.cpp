//====================================================
// power_mgr.cpp
// GPS ALWAYS ACTIVE VERSION
//====================================================
#include "power_mgr.h"
#include "display.h"
#include "comm/gsm.h"
#include "comm/location.h"
#include "config.h"
#include <Arduino.h>

//====================================================
// CONFIG & STATES & TIMERS
//====================================================
static const uint32_t DISPLAY_TIMEOUT_MS = 30000;
static const uint32_t GSM_WAKE_DELAY_MS = 15000, GSM_IDLE_TIMEOUT_MS = 5000;

static bool lockSleep = false, displayActive = false, forceDisplay = false;
static bool gpsRequested = true, gpsReady = true; // GPS luôn bật
static bool gsmRequested = false, gsmReady = false;

static uint32_t displayTimer = 0, motionTimer = 0;
static uint32_t gsmWakeTime = 0, gsmTimer = 0;

//====================================================
// INTERNAL HELPERS
//====================================================
static void wakeGPS()  { gpsRequested = true; gpsReady = true; }
static void sleepGPS() { /* Disable */ }

static void wakeGSM() {
    gsmTimer = millis();
    if(gsmRequested) return;
    Serial.println("[POWER] GSM WAKE");
    gsm_wakeup();
    gsmRequested = true; gsmReady = false;
    gsmWakeTime = gsmTimer;
}

static void sleepGSM() {
    if(!gsmRequested) return;
    Serial.println("[POWER] GSM SLEEP");
    gsm_sleep();
    gsmRequested = false; gsmReady = false;
}

//====================================================
// INTERFACE FUNCTIONS
//====================================================
void power_init() {
    lockSleep = false; displayActive = false; forceDisplay = false;
    gpsRequested = true; gpsReady = true;
    gsmRequested = false; gsmReady = false;

    display_sleep();
    location_wakeup(); // GPS INIT ONCE
    gsm_sleep();

    uint32_t now = millis();
    displayTimer = now; motionTimer = now; gsmTimer = now;
    Serial.println("[POWER] INIT\n[POWER] GPS ALWAYS ACTIVE");
}

void power_notifyButton() {
    display_wakeup(); displayActive = true;
    displayTimer = millis(); motionTimer = displayTimer;
    Serial.println("[POWER] BUTTON");
}

void power_notifyDisplay() {
    display_wakeup(); displayActive = true;
    displayTimer = millis();
}

void power_notifyMotion() { motionTimer = millis(); }

void power_notifyAlert() {
    lockSleep = true; forceDisplay = true;
    display_wakeup(); displayActive = true; displayTimer = millis();
    wakeGSM();
    Serial.println("[POWER] ALERT MODE");
}

void power_enterSafeMode() {
    forceDisplay = false; display_sleep(); displayActive = false;
    gsmTimer = millis() - GSM_IDLE_TIMEOUT_MS;
    Serial.println("[POWER] SAFE MODE");
}

void power_requestGPS()  { wakeGPS(); }
void power_releaseGPS()  { /* KEEP EMPTY */ }
void power_requestGSM()  { wakeGSM(); }
void power_releaseGSM()  { gsmTimer = millis(); }

void power_enableEmergencyGPS()  { Serial.println("[POWER] GPS ALWAYS ACTIVE"); }
void power_disableEmergencyGPS() { Serial.println("[POWER] GPS ALWAYS ACTIVE"); }

void power_lock()   { lockSleep = true; }
void power_unlock() { lockSleep = false; }

bool power_isLocked()       { return lockSleep; }
bool power_gpsReady()       { return true; }
bool power_gsmReady()       { return gsmReady; }
bool power_gpsActive()      { return true; }
bool power_gsmActive()      { return gsmRequested; }
bool power_displayActive()  { return displayActive; }

//====================================================
// UPDATE
//====================================================
void power_update() {
    uint32_t now = millis();

    gsm_update();
    location_update(); // GPS ALWAYS UPDATE

    // DISPLAY TIMEOUT
    if(displayActive && !forceDisplay && (now - displayTimer >= DISPLAY_TIMEOUT_MS)) {
        display_sleep(); displayActive = false;
        Serial.println("[POWER] DISPLAY TIMEOUT");
    }

    // GSM READY
    if(gsmRequested && !gsmReady && (now - gsmWakeTime >= GSM_WAKE_DELAY_MS)) {
        gsmReady = true;
        Serial.println("[POWER] GSM READY");
    }

    // GSM AUTO SLEEP
    if(gsmRequested && !lockSleep && (now - gsmTimer >= GSM_IDLE_TIMEOUT_MS)) {
        sleepGSM();
    }
}