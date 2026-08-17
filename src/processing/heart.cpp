//====================================================
// src/processing/heart.cpp
// MAX30102 - KIẾN TRÚC NO BUFFERS (XỬ LÝ DÒNG TỨC THỜI)
//====================================================

#include "heart.h"
#include <Arduino.h>
#include <math.h>
#include "heartRate.h"        // Giữ lại SparkFun: checkForBeat() để phát hiện nhịp tim

//====================================================
// CONFIG
//====================================================
#define MIN_BPM                 35
#define MAX_BPM                 220
#define WEAR_HOLD_MS            3000UL
#define NO_PEAK_TIMEOUT_MS      15000UL

#define PEAK_MIN_MS             273UL    // ~220 BPM max
#define PEAK_MAX_MS             1714UL   // ~35 BPM min

// Contact Detection (Kiểm tra áp ngón tay)
#define FINGER_IR_MIN           20000UL
#define FINGER_RED_MIN          20000UL
#define FINGER_RATIO_MIN        0.60f
#define FINGER_RATIO_MAX        1.60f
#define ADC_SATURATED           262000UL

// Cấu hình Bộ lọc dòng (EMA Coefficients)
#define DC_ALPHA                0.005f   // Bộ lọc lấy DC
#define AC_ALPHA                0.100f   // Bộ lọc làm mịn đỉnh AC

// Khởi tạo thông minh & Khóa thời gian
#define INITIAL_SPO2            97.5f    
#define LOCK_TIME_MS            6000UL   // Khóa màn hình trong 6 giây đầu

// BPM Smoothing
#define RATE_SIZE               6
#define BPM_SMOOTH_ALPHA        0.20f
#define BPM_MAX_JUMP            40.0f

//====================================================
// STATE (Trạng thái hệ thống)
//====================================================
enum PulseState { PULSE_NONE, PULSE_SEARCHING, PULSE_VALID };

static uint32_t g_irLast = 0, g_redLast = 0;
static bool     g_wearing = false, g_valid = false;
static float    g_bpm = 0.0f, g_lastGoodBpm = 0.0f;
static float    g_spo2 = 0.0f;
static bool     g_spo2Valid = false;

static uint32_t lastWearTime = 0;
static uint32_t firstContactTime = 0;
static uint32_t lastPeakTime = 0;
static uint32_t lastValidBeatTime = 0;
static PulseState pulseState = PULSE_NONE;

// Ring Buffer cho BPM
static float    rates[RATE_SIZE];
static uint8_t  rateSpot = 0, rateCount = 0;

// Các biến bộ lọc dòng tích lũy (No Buffers)
static float dcIr = 0.0f;
static float dcRed = 0.0f;
static float acIrMax = 0.0f, acIrMin = 0.0f;
static float acRedMax = 0.0f, acRedMin = 0.0f;
static float acIrMaxSmooth = 0.0f, acIrMinSmooth = 0.0f;
static float acRedMaxSmooth = 0.0f, acRedMinSmooth = 0.0f;

//====================================================
// HELPERS
//====================================================
static float medianOf(float *arr, int n) {
    float tmp[RATE_SIZE];
    for (int i = 0; i < n; i++) tmp[i] = arr[i];
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++)
            if (tmp[j] < tmp[i]) { float t = tmp[i]; tmp[i] = tmp[j]; tmp[j] = t; }
    return (n & 1) ? tmp[n/2] : (tmp[n/2-1] + tmp[n/2]) * 0.5f;
}

static void resetBpm() {
    rateSpot = rateCount = 0;
    for (int i = 0; i < RATE_SIZE; i++) rates[i] = 0.0f;
}

static void resetFilters(uint32_t initial_ir, uint32_t initial_red) {
    dcIr = (float)initial_ir;
    dcRed = (float)initial_red;
    acIrMax = acIrMaxSmooth = 100.0f;
    acIrMin = acIrMinSmooth = -100.0f;
    acRedMax = acRedMaxSmooth = 100.0f;
    acRedMin = acRedMinSmooth = -100.0f;
    g_spo2 = INITIAL_SPO2;
    g_spo2Valid = false;
}

static bool isFingerContact(uint32_t ir, uint32_t red) {
    if (ir >= ADC_SATURATED || red >= ADC_SATURATED) return false;
    if (ir < FINGER_IR_MIN || red < FINGER_RED_MIN)  return false;
    float ratio = (float)ir / (float)red;
    return (ratio >= FINGER_RATIO_MIN && ratio <= FINGER_RATIO_MAX);
}

//====================================================
// PUBLIC: INIT
//====================================================
void heart_init() {
    g_irLast = g_redLast = 0;
    g_wearing = g_valid = false;
    g_bpm = g_lastGoodBpm = 0.0f;
    lastWearTime      = millis();
    firstContactTime  = 0;
    lastPeakTime      = 0;
    lastValidBeatTime = millis();
    pulseState        = PULSE_NONE;
    resetBpm();
    resetFilters(0, 0);
    Serial.println("[Heart] Stream Processing Init OK (No Buffers)");
}

//====================================================
// PUBLIC: PROCESS — Gọi tức thời tại 50Hz khi có 1 mẫu
//====================================================
void heart_process(uint32_t ir_value, uint32_t red_value, bool /*hint*/, unsigned long now_ms) {
    g_irLast  = ir_value;
    g_redLast = red_value;

    //--------------------------------------------------
    // 1. CONTACT DETECTION
    //--------------------------------------------------
    bool fingerOK = isFingerContact(ir_value, red_value);

    if (fingerOK) {
        if (!g_wearing || firstContactTime == 0) {
            firstContactTime = now_ms;
            resetFilters(ir_value, red_value);
        }
        lastWearTime = now_ms;
        g_wearing    = true;
    } else {
        g_wearing = ((now_ms - lastWearTime) < WEAR_HOLD_MS);
    }

    if (!g_wearing) {
        if (g_valid || g_spo2Valid || g_bpm > 0) Serial.println("[Heart] Contact lost -> Reset");
        g_valid = false; g_bpm = 0.0f; g_lastGoodBpm = 0.0f;
        g_spo2 = 0.0f; g_spo2Valid = false;
        pulseState = PULSE_NONE;
        firstContactTime = 0; lastPeakTime = 0; lastValidBeatTime = now_ms;
        resetBpm();
        return;
    }

    //--------------------------------------------------
    // 2. BỘ LỌC TÍCH LŨY DÒNG TỨC THỜI (No Buffers)
    //--------------------------------------------------
    dcIr  = (dcIr  * (1.0f - DC_ALPHA)) + ((float)ir_value  * DC_ALPHA);
    dcRed = (dcRed * (1.0f - DC_ALPHA)) + ((float)red_value * DC_ALPHA);

    float acIrCurrent  = (float)ir_value - dcIr;
    float acRedCurrent = (float)red_value - dcRed;

    if (acIrCurrent > acIrMax) acIrMax = acIrCurrent;
    if (acIrCurrent < acIrMin) acIrMin = acIrCurrent;
    if (acRedCurrent > acRedMax) acRedMax = acRedCurrent;
    if (acRedCurrent < acRedMin) acRedMin = acRedCurrent;

    //--------------------------------------------------
    // 3. XỬ LÝ NHỊP TIM (SparkFun checkForBeat)
    //--------------------------------------------------
    bool beat = checkForBeat(ir_value);

    // Mức 1 & Mức 4: Cập nhật lastValidBeatTime tức thời và In log chẩn đoán [Beat]
    if (beat) {
        lastValidBeatTime = now_ms; 
        Serial.printf("[Beat] interval=%lu ms  IR=%lu\n", now_ms - lastPeakTime, ir_value);
        
        // --- CHU KỲ NHỊP TIM MỚI: TÍNH SPO2 VÀ BPM TẠI ĐÂY ---
        float acIrPp  = acIrMax - acIrMin;
        float acRedPp = acRedMax - acRedMin;

        acIrMaxSmooth  = (acIrMaxSmooth  * (1.0f - AC_ALPHA)) + (acIrPp  * AC_ALPHA);
        acRedMaxSmooth = (acRedMaxSmooth * (1.0f - AC_ALPHA)) + (acRedPp * AC_ALPHA);

        acIrMax = -10000.0f; acIrMin = 10000.0f;
        acRedMax = -10000.0f; acRedMin = 10000.0f;

        if (dcRed > 0 && dcIr > 0 && acIrMaxSmooth > 0) {
            float R = (acRedMaxSmooth / dcRed) / (acIrMaxSmooth / dcIr);
            float calculatedSpO2 = 103.41f - (7.42f * R);

            if(calculatedSpO2 > 99)  calculatedSpO2 = 99;
            if(calculatedSpO2 < 95)  calculatedSpO2 = 95;

            if (g_spo2 == INITIAL_SPO2) g_spo2 = calculatedSpO2;
            else g_spo2 = (g_spo2 * 0.8f) + (calculatedSpO2 * 0.2f);
        }

        // --- TÍNH TOÁN BPM ---
        if (lastPeakTime > 0) {
            uint32_t interval = (uint32_t)(now_ms - lastPeakTime);
            if (interval >= PEAK_MIN_MS && interval <= PEAK_MAX_MS) {
                float bpmRaw = 60000.0f / (float)interval;

                if (g_lastGoodBpm <= 0.0f || fabsf(bpmRaw - g_lastGoodBpm) <= BPM_MAX_JUMP) {
                    rates[rateSpot] = bpmRaw;
                    rateSpot = (rateSpot + 1) % RATE_SIZE;
                    if (rateCount < RATE_SIZE) rateCount++;

                    float med = medianOf(rates, rateCount);
                    if (g_lastGoodBpm <= 0.0f) g_lastGoodBpm = med;
                    else g_lastGoodBpm = (g_lastGoodBpm * (1.0f - BPM_SMOOTH_ALPHA)) + (med * BPM_SMOOTH_ALPHA);

                    g_bpm = g_lastGoodBpm;
                    if (rateCount >= 2) {
                        pulseState = PULSE_VALID; // Mức 1: Bỏ cập nhật time ở đây vì đã đưa lên đầu
                    }
                }
            }
        }
        lastPeakTime = now_ms;
    }

    //--------------------------------------------------
    // 4. BỘ KHÓA THỜI GIAN 6 GIÂY (bung số chuẩn)
    //--------------------------------------------------
    unsigned long timeElapsed = now_ms - firstContactTime;
    
    if (timeElapsed < LOCK_TIME_MS) {
        g_valid = false;
        g_spo2Valid = false;
    } else {
        if (pulseState == PULSE_VALID) g_valid = true;
        if (g_spo2 >= 85.0f && g_spo2 <= 100.0f) g_spo2Valid = true;
    }

    //--------------------------------------------------
    // 5. TIMEOUTS & STATE HOLDING
    //--------------------------------------------------
    // Giữ kết quả BPM trong 6 giây nếu mất nhịp tạm thời
    if (g_lastGoodBpm > 0.0f && (now_ms - lastValidBeatTime) < 6000UL) {
        if (timeElapsed >= LOCK_TIME_MS) g_bpm = g_lastGoodBpm;
    }

    // Nếu quá 2.5s không thấy đỉnh, chuyển sang trạng thái dò tìm nhịp
    if (lastPeakTime > 0 && (now_ms - lastPeakTime) > 2500) {
        pulseState = PULSE_SEARCHING;
    }

    // Mức 3: Nếu bỏ lỡ nhịp liên tiếp quá 4 giây thì reset bộ dò để tìm lại từ đầu
    if (lastPeakTime > 0 && (now_ms - lastPeakTime) > 4000) {
        resetBpm();
        lastPeakTime = 0;
        pulseState = PULSE_SEARCHING;
    }

    // Mức 2: Timeout tối đa (15 giây) - Chỉ xóa trắng dữ liệu khi thực sự mất tay
    if ((now_ms - lastValidBeatTime) > NO_PEAK_TIMEOUT_MS) {
        g_valid = false;
        g_spo2Valid = false;
        
        if (!g_wearing) {
            g_bpm = 0.0f;
            g_lastGoodBpm = 0.0f;
            g_spo2 = INITIAL_SPO2;
            resetBpm();
        }
        pulseState = PULSE_SEARCHING;
    }

    //--------------------------------------------------
    // 6. DEBUG LOG (1Hz)
    //--------------------------------------------------
    static uint32_t dbgT = 0;
    if (now_ms - dbgT >= 1000) {
        dbgT = now_ms;
        Serial.printf(
            "[Stream] Time:%lums | BPM=%.1f SpO2=%.1f%% | Valid_HR:%d Valid_SpO2:%d | State:%d\n",
            timeElapsed, g_bpm, g_spo2, g_valid, g_spo2Valid, (int)pulseState
        );
    }
}

//====================================================
// GETTERS
//====================================================
float heart_get_bpm()    { return g_bpm; }
bool  heart_is_valid()   { return g_valid; }
bool  heart_is_wearing() { return g_wearing; }
float heart_get_spo2()   { return g_spo2; }
bool  heart_spo2_valid() { return g_spo2Valid; }
void  heart_get_raw_values(uint32_t &ir_last, uint32_t &red_last) {
    ir_last  = g_irLast;
    red_last = g_redLast;
}
uint16_t heart_get_rr_interval() {
    return (uint16_t)(60000.0f / (g_bpm > 0 ? g_bpm : 75.0f));
}