#ifndef GSM_H
#define GSM_H

#include <Arduino.h>

// =========================
// INIT
// =========================
void gsm_init();
void gsm_update();

// =========================
// STATUS
// =========================
bool gsm_is_ready();

// =========================
// LOW LEVEL AT COMMAND
// =========================
bool gsm_sendAT(const char* cmd, const char* expect, unsigned long timeout);

// =========================
// SMS
// =========================
bool gsm_sendSMS(const char* phone, const char* message);

// =========================
// CALL EMERGENCY
// =========================
bool gsm_call(const char* phone);

// =========================
// EMERGENCY PACKAGE (FALL + HEALTH)
// =========================
bool gsm_sendEmergency(
    const char* phone,
    float lat,
    float lng,
    const char* message
);

// =========================
// POWER SAVE MODE
// =========================
// AT+CSCLK=2
void gsm_sleep();
void gsm_wakeup();

bool gsm_isSleeping();

bool gsm_powerLock();

bool gsm_powerUnlock();

bool gsm_isCallActive();

bool gsm_hangup();

#endif