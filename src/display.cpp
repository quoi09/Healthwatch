#include "display.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

Adafruit_SSD1306 oled(
    OLED_WIDTH,
    OLED_HEIGHT,
    &Wire,
    OLED_RESET_PIN
);

static bool displayActive = true;

//=======================================
// STATE
//=======================================
static uint32_t wearStart = 0;
static bool lastWearing = false;
#define WEAR_LOCK_MS 6000UL

//=======================================
// FALL TEXT
//=======================================
static const char* fall_state_to_string(FallState s) {
    switch(s) {
        case FALL_IDLE:      return "BINH THUONG";
        case FALL_VERIFY:    return "DANG KIEM TRA";
        case FALL_CONFIRMED: return "TE NGA";
        case FALL_CANCELLED: return "DA HUY";
        default:             return "---";
    }
}

//=======================================
// HEALTH TEXT
//=======================================
static const char* zone_to_string(health_zone_t z) {
    switch(z) {
        case ZONE_SAFE:    return "AN TOAN";
        case ZONE_WARNING: return "CANH BAO";
        case ZONE_DANGER:  return "NGUY HIEM";
        default:           return "--";
    }
}

//=======================================
// INIT
//=======================================
void display_init() {
    if(!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        DEBUG_PRINTLN("OLED FAIL");
        return;
    }
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    oled.setTextWrap(false);
    oled.display();
    DEBUG_PRINTLN("OLED OK");
}

void display_clear() { oled.clearDisplay(); }
void display_refresh() { oled.display(); }

//=======================================
// MAIN UPDATE
//=======================================
void display_update(const DisplayData &in)
{
    oled.clearDisplay();
    uint32_t now = millis();

    // phát hiện vừa đeo
    if(in.wearing && !lastWearing) {
        wearStart = now;
    }

    bool wearReady =
        in.wearing &&
        (now - wearStart >= WEAR_LOCK_MS);

    //-----------------------------------
    // FALL SCREEN
    //-----------------------------------
    if(in.fallState == FALL_CONFIRMED)
    {
        oled.drawRect(0, 0, 128, 64, SSD1306_WHITE);
        oled.setTextSize(1);
        oled.setCursor(16, 8);
        oled.println("BAN CO SAO KHONG?");
        oled.setCursor(12, 28);
        oled.println("NHAN NUT XAC NHAN");
        oled.setTextSize(2);
        oled.setCursor(56, 44);
        oled.print(in.fallCountdown);
        oled.display();
        lastWearing = in.wearing;
        return;
    }

    //-----------------------------------
    // ROW 1 : SPO2
    //-----------------------------------
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.print("SpO2:");

    if(wearReady && in.heartValid && in.spo2Valid) {
        oled.print((int)in.spo2);
        oled.print("%");
    } else {
        oled.print("--%");
    }

    //-----------------------------------
    // ROW 2 : BPM + ZONE
    //-----------------------------------
    oled.setCursor(0, 16);
    oled.print("BPM:");

    if(wearReady && in.heartValid) {
        oled.print((int)in.bpm);
    } else {
        if(in.wearing && in.noHeartTimeout && !in.spo2Valid) {
            oled.print("0");
        } else {
            oled.print("--");
        }
    }

    oled.setCursor(68, 16);
    oled.print(zone_to_string(in.zone));

    //-----------------------------------
    // ROW 3 : FALL STATUS
    //-----------------------------------
    oled.setCursor(0, 48);
    oled.print("TT:");
    oled.print(fall_state_to_string(in.fallState));

    if(in.fallState == FALL_VERIFY) {
        oled.setCursor(108, 48);
        oled.print("...");
    }

    oled.display();
    lastWearing = in.wearing;
}

//=======================================
// POWER
//=======================================
bool display_isActive() { return displayActive; }

void display_wakeup() {
    oled.ssd1306_command(SSD1306_DISPLAYON);
    displayActive = true;
    Serial.println("DISPLAY ON");
}

void display_sleep() {
    oled.ssd1306_command(SSD1306_DISPLAYOFF);
    displayActive = false;
    Serial.println("DISPLAY OFF");
}
