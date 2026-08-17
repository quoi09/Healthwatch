#include <Arduino.h>
#include <WiFi.h>
#include <system/power_mgr.h>
#include "services/mode.h"
#include <config.h>
#include <i2c_manager.h>
#include "drivers/max30102.h"
#include "drivers/mpu6500.h"
#include "drivers/button.h"
#include "processing/heart.h"
#include "processing/health.h"
#include "processing/motion.h"
#include "processing/fall.h"
#include "display.h"
#include "comm/gsm.h"
#include "comm/location.h"
#include "comm/firebase.h"
#include "services/alert.h"
#include "drivers/buzzer.h"

static uint32_t emergencyTrackStart = 0; static bool trackingEmergency = false;
static bool dangerPending = false; static bool waitingFirebaseFall = false; static bool waitingFirebaseDanger = false;
static uint32_t firebaseSyncStart = 0;

// New control flags for Buzzer & Firebase synchronization
static bool firebasePushPending = false; static bool buzzerWasOn = false;

// Heart rate Warmup & Metrics
bool heartWarmup = false; uint32_t heartWarmupStart = 0; const uint32_t HEART_WARMUP_MS = 10000;
DisplayData displayData; float ax, ay, az, gx, gy, gz; uint32_t lastMotionMs = 0;
bool sosSent = false; const char* EMERGENCY_PHONE = "+84848751140";
float g_bpm = 0; health_zone_t g_zone = ZONE_NONE;
static uint32_t lastHeartTime = 0; static uint32_t lastDisplay = 0; static uint32_t lastFirebase = 0;

void setup() {
    Serial.begin(9600); pinMode(BUTTON_PIN, INPUT_PULLUP);
    i2c_init(); mode_init(); mpu_init(); max_init(); button_init(); display_init(); gsm_init(); 
    location_init(); location_wakeup(); buzzer_init(); alert_init(EMERGENCY_PHONE); power_init(); delay(3000);
    
    Serial.println(max_is_ready() ? "MAX30102 OK" : "MAX30102 FAIL");
    if (!mpu_is_ready()) { Serial.println("MPU6500 FAIL"); } 
    else { Serial.print("WHOAMI: 0x"); Serial.println(mpu_get_whoami(), HEX); mpu_calibrate(); Serial.println("MPU6500 OK"); }
    Serial.println(gsm_is_ready() ? "GSM OK" : "GSM FAIL");
    
    heart_init(); health_init(); motion_init(); fall_init();
    Serial.println("[FIREBASE] INIT"); firebase_init(); Serial.println("[SYSTEM] READY");
    
    displayData.bpm = 0; displayData.spo2 = 0; displayData.spo2Valid = false;
    displayData.wearing = false; displayData.heartValid = false; displayData.zone = ZONE_NONE;
    displayData.fallState = FALL_IDLE; displayData.fallCountdown = 10; displayData.batteryPercent = 100;
    displayData.noHeartTimeout = false; lastMotionMs = millis();
    display_wakeup();    
}

void loop() {
    uint32_t now = millis(); button_update();

    if(button_short_press()) {
        power_notifyButton(); Serial.println("BUTTON SHORT"); buzzer_stop();
        if(displayData.fallState == FALL_CONFIRMED || displayData.fallState == FALL_VERIFY) {
            fall_cancel(); displayData.fallCountdown = 10; sosSent = false; firebasePushPending = false;
            power_disableEmergencyGPS(); power_enterSafeMode(); Serial.println("FALL CANCELLED"); buzzer_stop();
        }
    }

    //------------------------------------------------
    // MPU + FALL FSM
    //------------------------------------------------
    uint32_t dt = now - lastMotionMs; lastMotionMs = now;

    if (mpu_read(ax, ay, az, gx, gy, gz)) {
        motion_process(ax, ay, az, gx, gy, gz, dt);
        if (motion_is_moving()) power_notifyMotion();

        bool fallCandidate = motion_is_fall_candidate();
        FallState state = fall_update(fallCandidate); displayData.fallState = state;

        static uint32_t dbg = 0;
        if (now - dbg > 1000) {
            dbg = now;
            Serial.printf("fall=%d acc=%.2f gyro=%.1f orient=%.1f static=%d state=%d\n",
                fallCandidate, motion_get_peak_accel(), motion_get_peak_gyro(), motion_get_orientation_change(), motion_is_static(), state);
        }

        // FALL ALERT FLOW
        static bool fallTriggered = false; static int countdown = 10; static uint32_t countdownTick = 0;
        if (state == FALL_CONFIRMED) {
            buzzer_setMode(BUZZ_FALL);
            if (!fallTriggered) { fallTriggered = true; countdown = 10; countdownTick = millis(); power_notifyAlert(); Serial.println("FALL CONFIRMED"); }
            if (millis() - countdownTick >= 1000) { countdown--; countdownTick = millis(); }
            if (countdown < 0) countdown = 0; displayData.fallCountdown = countdown;

            if (countdown == 0 && !sosSent) {
                sosSent = true; firebasePushPending = true; Serial.println("WAIT FIREBASE PUSH");
            }
        } else {
            fallTriggered = false; countdown = 10; countdownTick = millis(); displayData.fallCountdown = 10;
            if (state == FALL_IDLE) sosSent = false;
        }
    }
    
    //================================================
    // REAL MODE (MAX30102)
    //================================================
    uint32_t ir = 0; uint32_t red = 0; bool wearing = false;
    if(max_read(ir, red, wearing)) {
        static uint32_t dbg = 0;
        if(millis() - dbg > 1000) { 
            dbg = millis(); 
            Serial.printf("IR=%u RED=%u wear=%d valid=%d BPM=%.1f SpO2=%.0f\n", ir, red, heart_is_wearing(), heart_is_valid(), heart_get_bpm(), heart_get_spo2()); 
        }
        
        displayData.wearing = heart_is_wearing();
        static float lastGoodBpm = 0; static uint32_t lastGoodTime = 0;
        bool validNow = heart_is_valid(); float bpmNow = heart_get_bpm();

        if(validNow) { lastHeartTime = millis(); }
        displayData.noHeartTimeout = (displayData.wearing && !validNow && (millis() - lastHeartTime > 7000));

        if(validNow && bpmNow > 30) { lastGoodBpm = bpmNow; lastGoodTime = millis(); }
        if(millis() - lastGoodTime < 10000) { displayData.heartValid = true; g_bpm = lastGoodBpm; } 
        else { displayData.heartValid = false; g_bpm = 0; }
        
        // DISPLAY CHỈ HIỆN SAU KHI ĐEO ĐỦ 7 GIÂY
        bool displayHealthReady = (displayData.wearing && (millis() - heartWarmupStart >= 7000));
        displayData.bpm = displayHealthReady ? g_bpm : 0;
        displayData.spo2 = (displayHealthReady && heart_spo2_valid()) ? heart_get_spo2() : 0;
        displayData.spo2Valid = (displayHealthReady && heart_spo2_valid());
        displayData.zone = displayHealthReady ? g_zone : ZONE_NONE;
        
        static bool lastWearing = false;
        if(displayData.wearing && !lastWearing) { heartWarmup = true; heartWarmupStart = millis(); Serial.println("[HEART] WARMUP START"); }
        if(!displayData.wearing) { 
            heartWarmup = false; lastGoodBpm = 0; lastGoodTime = 0; 
            displayData.heartValid = false; displayData.bpm = 0; displayData.noHeartTimeout = false; g_bpm = 0; 
        }
        lastWearing = displayData.wearing;
        
        if(heartWarmup && millis() - heartWarmupStart >= HEART_WARMUP_MS) { heartWarmup = false; Serial.println("[HEART] WARMUP DONE"); }
    }
    
    //------------------------------------------------
    // HEALTH ZONE (BPM=0 FORCE DANGER INTEGRATION)
    //------------------------------------------------
    if(displayData.wearing && !heartWarmup) {
        float healthBpm = displayData.heartValid ? g_bpm : heart_get_bpm();
        health_update(healthBpm, true, now); g_zone = health_get_zone();
    } else { g_zone = ZONE_NONE; }
    
    bool displayHealthReady = (displayData.wearing && (millis() - heartWarmupStart >= 7000));
    if(displayHealthReady) { displayData.bpm = g_bpm; displayData.zone = g_zone; } 
    else { displayData.bpm = 0; displayData.zone = ZONE_NONE; }
    displayData.fallState = fall_get_state();

    if(g_zone != ZONE_DANGER && buzzer_getMode() == BUZZ_DANGER) buzzer_stop();
    
    //================================================
    // REAL ALERT FLOW
    //================================================
    static bool warningPending = false; static bool warningLock = false; static bool dangerLock = false;
    if(health_warningTriggered() && !warningLock) { warningLock = true; warningPending = true; power_notifyDisplay(); power_requestGSM(); }
    if(!health_warningTriggered()) warningLock = false;
    if(warningPending && power_gsmReady() && !alert_isBusy()) { alert_sendHealthWarning(); health_clearWarning(); warningPending = false; Serial.println("REAL: WARNING SENT"); }
    
    if(health_dangerTriggered() && !dangerLock) { dangerLock = true; waitingFirebaseDanger = true; firebaseSyncStart = millis(); buzzer_setMode(BUZZ_DANGER); power_notifyAlert(); Serial.println("DANGER -> WAIT FIREBASE"); }
    if(!health_dangerTriggered()) dangerLock = false;
    if(dangerPending && power_gsmReady() && !alert_isBusy()) { alert_sendHealthDanger(); health_clearDanger(); dangerPending = false; Serial.println("REAL: DANGER SMS/CALL START"); }
    
    if(trackingEmergency && millis() - emergencyTrackStart >= 600000) { power_disableEmergencyGPS(); trackingEmergency = false; power_enterSafeMode(); Serial.println("[GPS] EMERGENCY TRACK END"); }
    
    alert_update(); 

    static bool lastBusy = false; bool busy = alert_isBusy();
    if(lastBusy && !busy) {
        Serial.println("ALERT DONE");
        if(sosSent) { Serial.println("GPS EMERGENCY -> FALL"); trackingEmergency = true; emergencyTrackStart = millis(); } 
        else if(g_zone == ZONE_DANGER) { Serial.println("GPS EMERGENCY -> DANGER"); } 
        else { power_enterSafeMode(); }
    }
    lastBusy = busy;
    
    //------------------------------------------------
    // DISPLAY UPDATE (EVERY 1S)
    //------------------------------------------------
    if(now - lastDisplay >= 1000) {
        lastDisplay = now;
        if(displayData.heartValid) { Serial.printf("[VALID] IR=%lu BPM=%.1f SpO2=%.0f wearing=%d zone=%d\n", ir, displayData.bpm, displayData.spo2, displayData.wearing, displayData.zone); } 
        else { Serial.printf("[INVALID] IR=%lu wear=%d timeout=%d\n", ir, displayData.wearing, displayData.noHeartTimeout); }
        display_update(displayData);
    }

    //------------------------------------------------
    // FIREBASE UPDATE & SYNC MANAGEMENT (EVERY 1S)
    //------------------------------------------------
    if(now - lastFirebase >= 1000) {
        lastFirebase = now;
        bool fallCounting = (displayData.fallState == FALL_CONFIRMED && displayData.fallCountdown > 0);

        if(!fallCounting) { // Hoãn push định kỳ nếu đang trong 10 giây đếm ngược
            FirebasePacket p;
            bool firebaseHealthReady = (displayData.wearing && (millis() - heartWarmupStart >= 7000));
            p.bpm = firebaseHealthReady ? (int)displayData.bpm : 0;
            p.spo2 = (firebaseHealthReady && heart_spo2_valid()) ? (int)heart_get_spo2() : 0;
            p.fallState = (displayData.fallState == FALL_CONFIRMED || sosSent);
            p.wearing = displayData.wearing ? "yes" : "no";

            if(firebaseHealthReady) {
                if(displayData.zone == ZONE_WARNING) p.zone = "warning";
                else if(displayData.zone == ZONE_DANGER) p.zone = "danger";
                else p.zone = "safe";
            } else { p.zone = "safe"; }

            static float lastLat = 0, lastLng = 0; float lat, lng, acc;
            if(location_getCachedGPS(lat, lng, acc)) { lastLat = lat; lastLng = lng; }
            p.gps.lat = lastLat; p.gps.lng = lastLng; p.datetime = "";

            // Lưu trạng thái và ngắt tạm thời còi trước khi ghi Firebase nhằm tránh giật tiếng loa (nếu cần)
            buzzerWasOn = (buzzer_getMode() != BUZZ_IDLE); 
            if(buzzerWasOn) buzzer_stop();

            firebase_update(p);

            // Khôi phục lại trạng thái còi ngay sau khi hoàn thành cập nhật dữ liệu
            if(buzzerWasOn) {
                if(displayData.fallState == FALL_CONFIRMED) buzzer_setMode(BUZZ_FALL);
                else if(g_zone == ZONE_DANGER) buzzer_setMode(BUZZ_DANGER);
            }

            // Kích hoạt đồng bộ hóa sau khi Firebase nhận dữ liệu khẩn cấp
            if(waitingFirebaseDanger && (millis() - firebaseSyncStart >= 1000)) { 
                waitingFirebaseDanger = false; dangerPending = true; power_requestGSM(); Serial.println("[FIREBASE] DANGER SYNC DONE"); 
            }
            if(firebasePushPending) {
                firebasePushPending = false; waitingFirebaseFall = true; firebaseSyncStart = millis(); Serial.println("[FIREBASE] FALL PUSH DONE");
            }
        }
    }

    // Luồng xử lý sau khi Firebase push gói khẩn cấp FALL xong -> chuyển giao sang GSM (trễ 1s)
    if(waitingFirebaseFall && (millis() - firebaseSyncStart >= 1000)) {
        waitingFirebaseFall = false; power_requestGSM(); Serial.println("[SYSTEM] START CALL/SMS SOS"); alert_triggerFallSOS();
    }

    buzzer_update(); power_update(); yield();
}