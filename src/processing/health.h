//====================================================
// health.h
// Health Monitoring Module
// BPM Zone Classification
//====================================================

#ifndef HEALTH_H
#define HEALTH_H

#include <Arduino.h>

//====================================================
// HEALTH ZONES
//====================================================

typedef enum {
    ZONE_NONE,

    ZONE_SAFE = 1,

    ZONE_WARNING,

    ZONE_DANGER

} health_zone_t;

//====================================================
// INIT
//====================================================

void health_init();

//====================================================
// MAIN UPDATE
//====================================================

// Update health state using BPM only
void health_update(float bpm,
                   bool wearing,
                   unsigned long now_ms);

//====================================================
// ZONE
//====================================================

// Determine zone directly
health_zone_t determineZone(float bpm,
                            bool wearing);

// Get current zone
health_zone_t health_get_zone();

// Get zone text
const char* health_get_zone_name();

//====================================================
// ALERT
//====================================================

// true when danger persists long enough
bool health_should_alert();

// reset danger alert
void health_reset_alert();

//====================================================
// TIME
//====================================================

// duration inside current zone
unsigned long health_get_zone_duration_ms();
bool health_warningTriggered();
bool health_dangerTriggered();

void health_clearWarning();
void health_clearDanger();

#endif