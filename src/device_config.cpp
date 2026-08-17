#include "device_config.h"
#include <Preferences.h>
#include <string.h>

static Preferences prefs;
DeviceConfig gConfig;

static void safeCopy(char* dst, const char* src, size_t size) {
    if (!dst || !src || size == 0) return;
    strncpy(dst, src, size - 1);
    dst[size - 1] = 0;
}

//========================
bool config_load() {
    prefs.begin("config", true);

    safeCopy(gConfig.wifiSsid, prefs.getString("ssid", "").c_str(), sizeof(gConfig.wifiSsid));
    safeCopy(gConfig.wifiPass, prefs.getString("pass", "").c_str(), sizeof(gConfig.wifiPass));
    safeCopy(gConfig.phone1,   prefs.getString("phone1", "").c_str(), sizeof(gConfig.phone1));
    safeCopy(gConfig.phone2,   prefs.getString("phone2", "").c_str(), sizeof(gConfig.phone2));

    prefs.end();
    return true;
}

//========================
bool config_save() {
    prefs.begin("config", false);

    prefs.putString("ssid",   gConfig.wifiSsid);
    prefs.putString("pass",   gConfig.wifiPass);
    prefs.putString("phone1", gConfig.phone1);
    prefs.putString("phone2", gConfig.phone2);

    prefs.end();
    return true;
}

//========================
bool config_reset() {
    prefs.begin("config", false);
    prefs.clear();
    prefs.end();

    memset(&gConfig, 0, sizeof(gConfig));
    return true;
}