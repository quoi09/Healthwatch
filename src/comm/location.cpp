//====================================================
// location.cpp
// GPS ALWAYS ACTIVE VERSION
//====================================================
#include "location.h"
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <Arduino.h>
#include <config.h>

//====================================================
// GPS & CACHE VARIABLES
//====================================================
static HardwareSerial gpsSerial(0);
static TinyGPSPlus gps;

static const uint32_t GPS_CACHE_TIMEOUT_MS = 9000000UL; // 150 phút
static float g_lat = 0.0f, g_lng = 0.0f, g_acc = 9999.0f;
static bool g_valid = false;
static uint32_t g_fixTime = 0;

//====================================================
// INTERNAL HELPERS
//====================================================
static void checkCacheTimeout() {
    if(!g_valid) return;

    if(millis() - g_fixTime >= GPS_CACHE_TIMEOUT_MS) {
        g_valid = false; g_lat = 0; g_lng = 0; g_acc = 9999;
        Serial.println("[GPS] CACHE EXPIRED");
    }
}

static void parseGPS() {
    // READ UART
    while(gpsSerial.available()) {
        gps.encode(gpsSerial.read());
    }

    // NEW FIX
    if(gps.location.isUpdated() && gps.location.isValid()) {
        float lat = gps.location.lat();
        float lng = gps.location.lng();
        float acc = gps.hdop.isValid() ? (gps.hdop.hdop() * 5.0f) : 9999.0f;

        // FILTER
        if(acc <= 100 || acc == 9999) {
            g_lat = lat; g_lng = lng; g_acc = acc; g_valid = true;
            g_fixTime = millis();

            /*Serial.println("[GPS] FIX");
            Serial.printf("LAT: %.6f\n", g_lat);
            Serial.printf("LNG: %.6f\n", g_lng);
            Serial.print("ACC: "); Serial.println(g_acc);*/
        }
    }
    static uint32_t dbg=0;
}

//====================================================
// INTERFACE FUNCTIONS
//====================================================
void location_init() {
    gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    g_valid = false;
    Serial.println("[GPS] INIT");
}

void location_update() {
    parseGPS();
    checkCacheTimeout();

    // In tọa độ mỗi 5000ms nếu dữ liệu hợp lệ
    static uint32_t lastPrintTime = 0;
    if (g_valid) {
        if (millis() - lastPrintTime >= 5000) {
            lastPrintTime = millis();
            Serial.printf("[GPS] LAT: %.6f | LNG: %.6f | ACC: %.1f\n", g_lat, g_lng, g_acc);
        }
    } else {
        lastPrintTime = 0; // Reset timer khi mất định vị
    }
}

bool location_getGPS(float& lat, float& lng, float& acc) {
    checkCacheTimeout();
    if(!g_valid) return false;

    lat = g_lat; lng = g_lng; acc = g_acc;
    return true;
}

bool location_getCachedGPS(float& lat, float& lng, float& acc) {
    return location_getGPS(lat, lng, acc);
}

uint32_t location_getCachedAge() {
    checkCacheTimeout();
    return g_valid ? (millis() - g_fixTime) : 0xFFFFFFFF;
}

// SLEEP & WAKE (DISABLED)
void location_sleep()   { Serial.println("[GPS] ALWAYS ACTIVE"); }
void location_wakeup()  { Serial.println("[GPS] ALWAYS ACTIVE"); }