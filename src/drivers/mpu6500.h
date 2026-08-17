//====================================================
// mpu6500.h
// Hardware Driver Layer
//====================================================

#ifndef MPU6500_H
#define MPU6500_H

#include <Arduino.h>

//----------------------------------------------------
// INIT / STATUS
//----------------------------------------------------

// Khởi tạo MPU6500
// - Accel ±8g
// - Gyro ±250dps
// - DLPF enabled
void mpu_init();

// Kiểm tra sensor hoạt động
bool mpu_is_ready();

// Đọc WHO_AM_I
uint8_t mpu_get_whoami();

//----------------------------------------------------
// CALIBRATION
//----------------------------------------------------

// Hiệu chuẩn accel + gyro offset
// Thiết bị phải nằm yên
void mpu_calibrate();

//----------------------------------------------------
// SENSOR READ
//----------------------------------------------------

// Đọc accel (g)
bool mpu_read_accel(
    float &ax,
    float &ay,
    float &az
);

// Đọc gyro (deg/s)
bool mpu_read_gyro(
    float &gx,
    float &gy,
    float &gz
);

// Đọc accel + gyro đồng thời
// accel: g
// gyro : deg/s
bool mpu_read(
    float &ax,
    float &ay,
    float &az,
    float &gx,
    float &gy,
    float &gz
);

//----------------------------------------------------
// POWER / INTERRUPT
//----------------------------------------------------

// Wake On Motion
// threshold: 1LSB ≈ 4mg
void mpu_setup_wom(uint8_t threshold);

// Clear interrupt flag
void mpu_clear_interrupt();

//----------------------------------------------------
// POWER MODES
//----------------------------------------------------

void mpu_motion_mode();
void mpu_normal_mode();

#endif