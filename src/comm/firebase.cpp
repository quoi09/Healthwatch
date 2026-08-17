#include "firebase.h"
#include "config.h"
#include "processing/health.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>

#define DEVICE_ID "device_01"
#include <math.h>

#define GPS_UPDATE_DISTANCE 30.0f   // mét

static float gpsDistanceMeters(float lat1, float lon1,
                               float lat2, float lon2)
{
    const float R = 6371000.0f;   // Earth radius (m)

    float dLat = radians(lat2 - lat1);
    float dLon = radians(lon2 - lon1);

    float a =
        sin(dLat * 0.5f) * sin(dLat * 0.5f) +
        cos(radians(lat1)) *
        cos(radians(lat2)) *
        sin(dLon * 0.5f) *
        sin(dLon * 0.5f);

    float c = 2.0f * atan2(sqrt(a), sqrt(1.0f - a));

    return R * c;
}

static bool lastWearing = false;
static int lastBpm = -1; static int lastSpo2 = -1;
static float lastLat = 0; static float lastLng = 0;
static uint32_t lastHealthUpload = 0;

//=====================================================
// GLOBAL & WIFI
//=====================================================
static uint32_t lastUpload = 0;
static bool wifiTried = false;
static String lastZone = ""; static bool lastFall = false;

bool firebase_is_connected() { return WiFi.status() == WL_CONNECTED; }

void wifi_connect() {
    if (wifiTried) return;
    wifiTried = true;

#if DEBUG_SERIAL
    DEBUG_PRINTLN("Connecting WiFi");
#endif
    WiFi.mode(WIFI_STA); WiFi.setTxPower(WIFI_POWER_8_5dBm);

    const char* ssidList[] = { YOUR_WIFI, YOUR_WIFI2 };
    const char* passList[] = { YOUR_PASS, YOUR_PASS2 };
    bool connected = false;

    for (int i = 0; i < 2; i++) {
#if DEBUG_SERIAL
        DEBUG_PRINT("Try WiFi "); DEBUG_PRINT(i + 1); DEBUG_PRINT(": "); DEBUG_PRINTLN(ssidList[i]);
#endif
        WiFi.disconnect(true); yield();
        WiFi.begin(ssidList[i], passList[i]);

        uint32_t start = millis(); uint32_t dot = millis();

        while (WiFi.status() != WL_CONNECTED) {
            if (millis() - start > 5000) break;
#if DEBUG_SERIAL
            if (millis() - dot > 500) { DEBUG_PRINT("."); dot = millis(); }
#endif
            yield();
        }

        if (WiFi.status() == WL_CONNECTED) {
            connected = true;
#if DEBUG_SERIAL
            DEBUG_PRINTLN(""); DEBUG_PRINT("Connected: "); DEBUG_PRINTLN(ssidList[i]);
            DEBUG_PRINT("IP: "); DEBUG_PRINTLN(WiFi.localIP());
#endif
            break;
        }
#if DEBUG_SERIAL
        DEBUG_PRINTLN(""); DEBUG_PRINTLN("Connect fail");
#endif
    }

    if (!connected) {
#if DEBUG_SERIAL
        DEBUG_PRINTLN("All WiFi failed");
#endif
        return;
    }

#if DEBUG_SERIAL
    DEBUG_PRINTLN("Sync NTP");
#endif
    configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    struct tm timeinfo; uint32_t ntpStart = millis();

    while (!getLocalTime(&timeinfo)) {
        if (millis() - ntpStart > 10000) {
#if DEBUG_SERIAL
            DEBUG_PRINTLN("TIME FAIL");
#endif
            return;
        }
        yield();
    }
#if DEBUG_SERIAL
    DEBUG_PRINTLN("TIME OK");
#endif
}

//=====================================================
// INIT & TIME & URL
//=====================================================
void firebase_init() {
#if DEBUG_SERIAL
    DEBUG_PRINTLN("[FIREBASE] START");
#endif
    wifi_connect();
}

static String timestamp() { return String(millis()); }

static String datetimeString() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return "NoTime";
    char buf[32]; strftime(buf, sizeof(buf), "%d-%m-%Y %H:%M:%S", &timeinfo);
    return String(buf);
}

static String firebase_url(const char* path) {
    return String("https://") + FIREBASE_HOST + "/" + path + ".json?auth=" + FIREBASE_AUTH;
}

//=====================================================
// HTTPS & JSON
//=====================================================
static int firebase_put(const String& url, const String& payload) {
    if (!firebase_is_connected()) return -1;

    WiFiClientSecure client;
    client.setInsecure(); client.setTimeout(10000);

    HTTPClient https; int code = -1;
    if (https.begin(client, url)) {
        https.addHeader("Content-Type", "application/json");
        code = https.PUT(payload);
        https.end();
    }
    return code;
}

static String buildPayload(const FirebasePacket& data) {
    String s; s.reserve(512);
    s += "{";
    s += "\"datetime\":\"" + String(data.datetime.length() ? data.datetime : datetimeString()) + "\"";
    s += ",\"bpm\":" + String((data.wearing == "yes") ? String(data.bpm) : "0");
    s += ",\"spo2\":" + String(data.spo2);
    s += ",\"fallState\":" + String(data.fallState ? "true" : "false");
    s += ",\"wearing\":\"" + data.wearing + "\"";
    s += ",\"zone\":\"" + data.zone + "\"";
    s += ",\"gps\":{\"lat\":" + String(data.gps.lat, 6) + ",\"lng\":" + String(data.gps.lng, 6) + "}";
    s += "}";
    return s;
}

//=====================================================
// PUSH & UPDATE
//=====================================================
bool firebase_pushRealtime(const FirebasePacket& data) {
    String payload = buildPayload(data); Serial.println(payload);
    String path = String(DEVICE_ID) + "/realtime";
    int code = firebase_put(firebase_url(path.c_str()), payload);
    Serial.printf("HTTP=%d\n", code);
    return (code == 200 || code == 201);
}

bool firebase_pushHistory(const FirebasePacket& data) {
    String path = String(DEVICE_ID) + "/history/" + timestamp();
    return firebase_put(firebase_url(path.c_str()), buildPayload(data)) > 0;
}

void firebase_update(const FirebasePacket& data) {
    bool zoneChanged = (lastZone != data.zone);
    bool fallChanged = (lastFall != data.fallState);
    bool wearingChanged = (lastWearing != (data.wearing == "yes"));
    bool gpsChanged = false;

if (data.gps.lat != 0.0f && data.gps.lng != 0.0f)
{
    // Chưa từng upload GPS
    if (lastLat == 0.0f && lastLng == 0.0f)
    {
        gpsChanged = true;
    }
    else
    {
        float distance =
            gpsDistanceMeters(lastLat, lastLng,
                              data.gps.lat, data.gps.lng);

        gpsChanged = (distance >= GPS_UPDATE_DISTANCE);

#if DEBUG_SERIAL
        Serial.printf("[GPS] Move = %.1f m\n", distance);
#endif
    }
}
    bool bpmChanged = (lastBpm != data.bpm);
    bool spo2Changed = (lastSpo2 != data.spo2);

    bool uploadNow = zoneChanged || fallChanged || wearingChanged || gpsChanged;
    bool healthChanged = bpmChanged || spo2Changed;

    if (healthChanged) {
        if (zoneChanged) { uploadNow = true; } 
        else if (millis() - lastHealthUpload >= 10000) { uploadNow = true; }
    }

    if (uploadNow) {
        bool ok = firebase_pushRealtime(data);
        if (ok) {
            lastUpload = millis(); lastHealthUpload = millis();
            lastZone = data.zone; lastFall = data.fallState;
            lastWearing = (data.wearing == "yes");
            lastBpm = data.bpm; lastSpo2 = data.spo2;
            lastLat = data.gps.lat; lastLng = data.gps.lng;
        }
    }
}