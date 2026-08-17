//====================================================
// src/processing/health.cpp
//====================================================
#include "health.h"
#include "../services/mode.h"

//====================================================
// BPM ZONE THRESHOLDS
//====================================================
// SAFE: 50 - 100 BPM (Người già nhịp tim hơi chậm là bình thường, nhưng không nên quá cao) màu xanh flutter
#define BPM_SAFE_MIN          50
#define BPM_SAFE_MAX          100

// WARNING: 40 - 49 BPM (Cảnh báo chậm) | 101 - 110 BPM (Cảnh báo nhanh) // màu vàng flutter
#define BPM_WARN_LOW_MIN      40
#define BPM_WARN_LOW_MAX      49
#define BPM_WARN_HIGH_MIN     101
#define BPM_WARN_HIGH_MAX     110

// DANGER: < 40 BPM (Quá chậm, nguy cơ ngất) | > 110 BPM (Quá nhanh đối với tim người già)// màu đỏ flutter
#define BPM_DANGER_LOW_MAX    39
#define BPM_DANGER_HIGH_MIN   111

//====================================================
// ALERT CONFIG
//====================================================
#define WARNING_ALERT_DELAY_MS   20000UL // WARNING after 20s
#define DANGER_ALERT_DELAY_MS    5000UL  // DANGER after 5s
#define WEAR_HOLD_MS             3000UL  // remove wear flicker

//====================================================
// STATE & ALERT FLAGS
//====================================================
static health_zone_t current_zone = ZONE_SAFE;
static unsigned long zone_enter_time = 0, last_wear_time = 0;
static bool warning_triggered = false, danger_triggered = false;

//====================================================
// INIT
//====================================================
void health_init() {
    Serial.println("[Health] Initializing");
    current_zone = ZONE_SAFE;
    zone_enter_time = millis(); last_wear_time = millis();
    warning_triggered = false; danger_triggered = false;
}

//====================================================
// DETERMINE ZONE
//====================================================
health_zone_t determineZone(float bpm, bool wearing) {
    //------------------------------------------------
    // WEAR HOLD & NOT WEARING
    //------------------------------------------------
    if (wearing) { last_wear_time = millis(); }
    else if (millis() - last_wear_time < WEAR_HOLD_MS) { wearing = true; } // ignore short wear loss

    if (!wearing) return ZONE_SAFE;

    //------------------------------------------------
    // BƯỚC 4: FORCE DANGER KHI MẤT NHỊP HOÀN TOÀN (BPM <= 0)
    //------------------------------------------------
    if (bpm <= 0) {
        return ZONE_DANGER;
    }

    //------------------------------------------------
    // CHECK ZONES (SAFE / DANGER / WARNING)
    //------------------------------------------------
    if (bpm >= BPM_SAFE_MIN && bpm <= BPM_SAFE_MAX) return ZONE_SAFE;
    if (bpm <= BPM_DANGER_LOW_MAX || bpm >= BPM_DANGER_HIGH_MIN) return ZONE_DANGER;
    if ((bpm >= BPM_WARN_LOW_MIN && bpm <= BPM_WARN_LOW_MAX) || (bpm >= BPM_WARN_HIGH_MIN && bpm <= BPM_WARN_HIGH_MAX)) return ZONE_WARNING;

    return ZONE_SAFE;
}

//====================================================
// UPDATE
//====================================================
void health_update(float bpm, bool wearing, unsigned long now_ms) {
    //------------------------------------------------
    // DETERMINE NEW ZONE & ZONE TRANSITION
    //------------------------------------------------
    health_zone_t new_zone = determineZone(bpm, wearing);

    if (new_zone != current_zone) {
        const char* zone_names[] = {"NONE", "SAFE", "WARNING", "DANGER"};
        Serial.print("[Health] "); Serial.print(zone_names[current_zone]);
        Serial.print(" -> "); Serial.println(zone_names[new_zone]);

        current_zone = new_zone;
        zone_enter_time = now_ms;
        warning_triggered = false; danger_triggered = false;
    }

    //------------------------------------------------
    // TIME IN ZONE & ALERTS TRIGGER
    //------------------------------------------------
    unsigned long zone_time = now_ms - zone_enter_time;

    if (current_zone == ZONE_WARNING &&
        zone_time >= getWarningDelay() &&
        !warning_triggered)
    {
        warning_triggered = true;
        Serial.print("[Health] WARNING triggered | delay="); Serial.println(getWarningDelay());
    }

    if (current_zone == ZONE_DANGER &&
        zone_time >= getDangerDelay() &&
        !danger_triggered)
    {
        danger_triggered = true;
        Serial.print("[Health] DANGER triggered | delay="); Serial.println(getDangerDelay());
    }

    //------------------------------------------------
    // DEBUG LOG
    //------------------------------------------------
    static unsigned long last_debug = 0;
    if (now_ms - last_debug >= 1000) {
        last_debug = now_ms;
        Serial.print("[Health] BPM:"); Serial.print(bpm, 1);
        Serial.print(" | Zone:"); Serial.print((int)current_zone);
        Serial.print(" | Time:"); Serial.print(zone_time / 1000); Serial.print("s");
        Serial.print(" | Warning:"); Serial.print(warning_triggered);
        Serial.print(" | Danger:"); Serial.println(danger_triggered);
    }
}

//====================================================
// GETTERS & SETTERS (WARNING / DANGER / LEGACY)
//====================================================
health_zone_t health_get_zone() { return current_zone; }

const char* health_get_zone_name() {
    switch (current_zone) {
        case ZONE_NONE:    return "--";
        case ZONE_SAFE:    return "SAFE";
        case ZONE_WARNING: return "WARN";
        case ZONE_DANGER:  return "DANGER";
        default:           return "UNKNOWN";
    }
}

bool health_warningTriggered() { return warning_triggered; }
void health_clearWarning() { warning_triggered = false; }

bool health_dangerTriggered() { return danger_triggered; }
void health_clearDanger() { danger_triggered = false; }

bool health_should_alert() { return danger_triggered; }
void health_reset_alert() { danger_triggered = false; }
unsigned long health_get_zone_duration_ms() { return millis() - zone_enter_time; }