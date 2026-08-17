#ifndef CONFIG_H
#define CONFIG_H

// WIFI
#define YOUR_WIFI        "SS"
#define YOUR_PASS        "12345678"

#define YOUR_WIFI2 "Wifi New"
#define YOUR_PASS2 "1122@1122"

#define USER_EMAIL "app1@email.com"
#define USER_PASSWORD "123456"


// ---------- WAKE / INTERRUPT (Vùng RTC an toàn) ----------
#define BUTTON_PIN       1     
#define MPU_INT_PIN      2     

// ---------- I2C ----------
#define SDA_PIN          4
#define SCL_PIN          5
#define I2C_FREQ         100000

// ---------- OUTPUT ----------
#define BUZZER_PIN       10

// ---------- OPTIONAL ----------
#define MAX_INT_PIN      3     // Vẫn nằm trong vùng RTC, an toàn

// ---------- GSM ----------
#define GSM_RX_PIN       6
#define GSM_TX_PIN       7
#define GSM_DTR_PIN      8
#define GSM_BAUDRATE     115200

// ---------- GPS ----------
#define GPS_RX_PIN       20
#define GPS_TX_PIN       21
#define GPS_BAUD         9600

// OLED RESET (nếu cần)
#define OLED_RESET_PIN   -1

// =========================
// ❤️ HEART SENSOR CONFIG (MAX30102)
// =========================

// Sampling
#define HR_SAMPLE_RATE       25
#define HR_BUFFER_SIZE       100

// Detect đeo tay
#define IR_THRESHOLD_WEAR    20000

// =========================
// 📊 SPO2 + HEART RATE ZONE
// =========================

// -------- ZONE 1 (SAFE)
#define SPO2_SAFE_MIN        96
#define BPM_SAFE_MIN         60
#define BPM_SAFE_MAX         90

// -------- ZONE 2 (WARNING)
#define SPO2_WARN_MIN        90
#define SPO2_WARN_MAX        95

#define BPM_WARN_LOW_MIN     45
#define BPM_WARN_LOW_MAX     59

#define BPM_WARN_HIGH_MIN    91
#define BPM_WARN_HIGH_MAX    120

// -------- ZONE 3 (DANGER)
#define SPO2_DANGER_MAX      89
#define BPM_DANGER_LOW_MAX   40
#define BPM_DANGER_HIGH_MIN  121

// =========================
// ⏱️ TIMING
// =========================

// debounce button
#define BUTTON_DEBOUNCE_MS       200

// fall detect timeout
#define FALL_CONFIRM_TIME_MS     3000
#define FALL_ALERT_DELAY_MS      10000

// lọc nhiễu HR
#define HR_STABLE_TIME_MS        3000

// gửi Firebase
#define FIREBASE_INTERVAL_NORMAL 300000
#define FIREBASE_INTERVAL_ALERT  5000

// =========================
// 🚨 MPU6500 + FALL DETECTION
// =========================

// IMU sample rate
#define MOTION_SAMPLE_RATE_HZ       100
#define MOTION_DT_MS                10


#define FALL_JERK_THRESHOLD         10.0f
// ====================================================================
// 1. TẦNG VA CHẠM (IMPACT DETECTION)
// ====================================================================
// Gia tốc tuyến tính phải > 2.2g VÀ đỉnh va chạm phải > 2.5g (trong fall.cpp tự cộng 0.3f)
#define FALL_ACC_THRESHOLD          1.8f

#define FREE_FALL_THRESHOLD         0.65f

#define FALL_GYRO_THRESHOLD         100.0f

#define FALL_ORIENTATION_THRESHOLD  45.0f

#define STATIC_ACC_THRESHOLD        0.15f

#define STATIC_GYRO_THRESHOLD       20.0f

#define STATIC_TIME_MS              2500

// Complementary filter
#define COMP_FILTER_ALPHA           0.98f

// Motion processing buffer
#define MOTION_BUFFER_SIZE          32

// Wake-on-motion threshold
// 1LSB ≈ 4mg
#define WOM_THRESHOLD               10

// MPU calibration samples
#define MPU_CALIB_SAMPLES           200

// =========================
// 📡 GPS + WIFI LOCATION
// =========================

// timeout GPS fix
#define GPS_TIMEOUT_MS        10000

// wifi scan count
#define WIFI_SCAN_MAX         10

// =========================
// 🔋 POWER MANAGEMENT
// =========================

// deep sleep interval (normal)
#define DEEP_SLEEP_TIME_SEC   60

// wakeup pin
#define WAKEUP_BUTTON_PIN     BUTTON_PIN

// =========================
// 📱 GSM CONFIG
// =========================

#define PHONE_NUMBER "0848751140"

// AT command delay
#define GSM_CMD_DELAY_MS  3000

// =========================
// 🌐 FIREBASE CONFIG
// =========================

#define FIREBASE_HOST "health-monitoring-system-78d19-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "AIzaSyBVfAqssG_aYHaYjeNT58EVctk4vKpmHCg"

#define FEATURE_SIZE  6

// =========================
// 📺 OLED CONFIG
// =========================

#define OLED_WIDTH   128
#define OLED_HEIGHT  64

// =========================
// 🛡️ DEBUG
// =========================

#define DEBUG_SERIAL 1

#if DEBUG_SERIAL
  #define DEBUG_PRINT(...)   Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif

#endif