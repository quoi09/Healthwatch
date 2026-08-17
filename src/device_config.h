#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

struct DeviceConfig {
    char wifiSsid[64];
    char wifiPass[64];
    char phone1[24];
    char phone2[24];
};

extern DeviceConfig gConfig;

bool config_load();
bool config_save();
bool config_reset();

#endif