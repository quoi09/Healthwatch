#include "config.h"
#include <Arduino.h>
#include <TinyGPS++.h>

extern void display_showTempLine(const char* line);

static TinyGPSPlus gps;
static HardwareSerial GPSSerial(2);

static bool gpsStarted = false;

void neo_init()
{
    if (gpsStarted) return;

    GPSSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    delay(300);

    Serial.println("[GPS] Neo6M started");

    gpsStarted = true;
}

bool neo_read(double &lat, double &lon, String &datetime, bool &hasFix) {
    // 1. Kiểm tra xem có nhận được dữ liệu thô nào không (Lỗi phần cứng)
    if (millis() > 5000 && gps.charsProcessed() < 10) {
        datetime = "HW ERROR: Check RX/TX";
        hasFix = false;
        return false;
    }

    // 2. Kiểm tra xem đã chốt được vị trí chưa
    if (gps.location.isValid()) {
        hasFix = true;
        lat = gps.location.lat();
        lon = gps.location.lng();

        // Định dạng thời gian
        if (gps.date.isValid() && gps.time.isValid()) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%02d/%02d %02d:%02d", 
                     gps.date.day(), gps.date.month(), 
                     gps.time.hour(), gps.time.minute());
            datetime = String(buf);
        }
        return true;
    } 
    
    // 3. Nếu chưa có Fix, trả về thông tin vệ tinh để in lỗi chi tiết
    hasFix = false;
    int sats = gps.satellites.value();
    if (sats == 0) {
        datetime = "Searching Sats...";
    } else {
        char buf[32];
        snprintf(buf, sizeof(buf), "Signal Weak (%d Sats)", sats);
        datetime = String(buf);
    }
    
    return false;
}
// Thêm hàm này để chạy liên tục trong loop()
void neo_update() {
    while (GPSSerial.available()) {
        gps.encode(GPSSerial.read());
    }
}
