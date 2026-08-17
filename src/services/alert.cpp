//====================================================
// alert.cpp
//====================================================
#include "alert.h"
#include "mode.h"
#include "comm/gsm.h"
#include "comm/location.h"
#include "system/power_mgr.h"
#include <Arduino.h>

static const char* g_phone = nullptr;
static uint32_t lastSmsMs = 0;
static uint32_t lastCallMs = 0;

static float gpsLat = 0;
static float gpsLng = 0;
static float gpsAcc = 0;

enum AlertType { ALERT_NONE, ALERT_WARNING, ALERT_DANGER, ALERT_FALL };
enum AlertState { ALERT_IDLE, ALERT_CALL, ALERT_WAIT_CALL, ALERT_GET_GPS, ALERT_SMS, ALERT_ENABLE_TRACKING, ALERT_DONE };

static AlertType currentType = ALERT_NONE;
static AlertState state = ALERT_IDLE;

static bool canSendSMS() { return (millis() - lastSmsMs) >= getSmsCooldown(); }
static bool canCall() { return (millis() - lastCallMs) >= getCallCooldown(); }

void alert_init(const char* emergencyPhone) { g_phone = emergencyPhone; }
bool alert_isBusy() { return state != ALERT_IDLE; }

static void requestAlert(AlertType type, AlertState startState, const char* msg) {
    if (!g_phone) return;
    if (alert_isBusy()) { Serial.println("[ALERT] BUSY"); return; }
    if (type == ALERT_WARNING && !canSendSMS()) { Serial.println("[ALERT] WARNING SMS COOLDOWN"); return; }

    Serial.println("================================");
    Serial.print("[ALERT] "); Serial.print(msg); Serial.println(" REQUEST");
    Serial.println("================================");

    currentType = type;
    state = startState;
}

void alert_sendHealthWarning() { requestAlert(ALERT_WARNING, ALERT_SMS, "WARNING"); }
void alert_sendHealthDanger() { requestAlert(ALERT_DANGER, ALERT_CALL, "DANGER"); }
void alert_triggerFallSOS() { requestAlert(ALERT_FALL, ALERT_CALL, "FALL SOS"); }

void alert_update() {
    static uint32_t dbg = 0;
    if (millis() - dbg >= 2000) {
        dbg = millis();
        Serial.print("[ALERT FSM] state="); Serial.println(state);
    }

    switch (state) {
        case ALERT_IDLE: break;

        case ALERT_CALL: {
            if (!canCall()) break;
            Serial.println("[ALERT] CALL");
            bool ok = gsm_call(g_phone);
            Serial.print("[ALERT] CALL RESULT="); Serial.println(ok);

            if (ok) { lastCallMs = millis(); state = ALERT_WAIT_CALL; }
            else { Serial.println("[ALERT] CALL FAILED"); state = ALERT_DONE; }
            break;
        }
        case ALERT_WAIT_CALL: {
            if (!gsm_isCallActive()) {
                Serial.println("[ALERT] CALL FINISHED");
                gsm_hangup();
                delay(1000);
                state = ALERT_GET_GPS;
            }
            break;
        }
        case ALERT_GET_GPS: {
            if (location_getCachedGPS(gpsLat, gpsLng, gpsAcc)) {
                Serial.println("[ALERT] GPS CACHE OK");
                Serial.print("LAT: "); Serial.println(gpsLat, 6);
                Serial.print("LNG: "); Serial.println(gpsLng, 6);
                Serial.print("ACC: "); Serial.println(gpsAcc);
                Serial.print("AGE(ms): "); Serial.println(location_getCachedAge());
            } else {
                gpsLat = 10.852167f; gpsLng = 106.772000f; gpsAcc = 5.0f;
                Serial.println("[ALERT] USING DEMO GPS");
            }
            state = ALERT_SMS;
            break;
        }
        case ALERT_SMS: {
            if (!canSendSMS()) break;
            bool ok = false;

            if (currentType == ALERT_WARNING) {
                Serial.println("[ALERT] WARNING SMS");
                ok = gsm_sendSMS(g_phone, "CANH BAO: Nhip tim bat thuong.");
            } else if (currentType == ALERT_DANGER) {
                Serial.println("[ALERT] DANGER SMS");
                ok = gsm_sendEmergency(g_phone, gpsLat, gpsLng, "NGUY HIEM: Nhip tim nguy hiem");
            } else if (currentType == ALERT_FALL) {
                Serial.println("[ALERT] FALL SMS");
                ok = gsm_sendEmergency(g_phone, gpsLat, gpsLng, "PHAT HIEN TE NGA");
            }

            Serial.print("[ALERT] SMS RESULT="); Serial.println(ok);
            lastSmsMs = millis();

            if (ok) state = (currentType == ALERT_DANGER || currentType == ALERT_FALL) ? ALERT_ENABLE_TRACKING : ALERT_DONE;
            else { Serial.println("[ALERT] SMS FAILED"); state = ALERT_DONE; }
            break;
        }
        case ALERT_ENABLE_TRACKING: {
            Serial.println("[ALERT] ENABLE EMERGENCY GPS");
            power_enableEmergencyGPS();
            state = ALERT_DONE;
            break;
        }
        case ALERT_DONE: {
            Serial.println("[ALERT] DONE");
            currentType = ALERT_NONE;
            state = ALERT_IDLE;
            break;
        }
        default: break;
    }
}