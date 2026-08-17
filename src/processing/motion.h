#pragma once

#include <stdint.h>
#include <stdbool.h>

void motion_init();

void motion_process(
    float ax,
    float ay,
    float az,
    float gx,
    float gy,
    float gz,
    unsigned long dt_ms
);

// accel
float motion_get_accel_magnitude();
float motion_get_linear_accel();
float motion_get_peak_accel();
float motion_get_stddev();

// gyro
float motion_get_gyro_magnitude();
float motion_get_peak_gyro();

// orientation
float motion_get_pitch();
float motion_get_roll();
float motion_get_orientation_change();

void motion_reset_baseline();

// states
bool motion_is_static();
bool motion_is_moving();
bool motion_is_free_fall();

// fall
bool motion_is_fall_candidate();