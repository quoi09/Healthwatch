//====================================================
// motion.cpp
//====================================================
#include "motion.h"
#include "config.h"
#include <Arduino.h>
#include <math.h>

static const float RAD2DEG = 57.2957795f;
static const float LP = 0.02f;
static const uint32_t PEAK_HOLD = 300;
static const uint32_t EVENT_HOLD = 1500;

//----------------------------------------------------
// STATE
//----------------------------------------------------
static float grav_x=0, grav_y=0, grav_z=0;
static float pitch=0, roll=0;
static float baseline_pitch=0, baseline_roll=0;
static bool baseline_set=false;

static float accel_mag=0, linear_mag=0, gyro_mag=0;
static float peak_accel=0, peak_gyro=0;
static uint32_t peak_accel_time=0, peak_gyro_time=0;

static float mean_lin=0, mean_gyro=0;
static bool static_state=false;
static bool last_static=false;
static uint32_t static_ms=0;

//----------------------------------------------------
// FALL EVENT CACHE
//----------------------------------------------------
static uint32_t impact_time=0;
static uint32_t rotate_time=0;
static uint32_t orient_time=0;

//====================================================

void motion_init() {
    grav_x=grav_y=grav_z=0;
    pitch=roll=0;
    baseline_pitch=baseline_roll=0;
    baseline_set=false;

    accel_mag=linear_mag=gyro_mag=0;
    peak_accel=peak_gyro=0;
    peak_accel_time=peak_gyro_time=0;

    mean_lin=mean_gyro=0;
    static_state=false;
    last_static=false;
    static_ms=0;

    impact_time=0;
    rotate_time=0;
    orient_time=0;
}

//====================================================

void motion_process(float ax,float ay,float az,
                    float gx,float gy,float gz,
                    unsigned long dt_ms) {

    float dt = dt_ms * 0.001f;
    uint32_t now = millis();

    //------------------------------------------------
    // GRAVITY
    //------------------------------------------------
    grav_x=(1-LP)*grav_x + LP*ax;
    grav_y=(1-LP)*grav_y + LP*ay;
    grav_z=(1-LP)*grav_z + LP*az;

    float lax=ax-grav_x;
    float lay=ay-grav_y;
    float laz=az-grav_z;

    //------------------------------------------------
    // MAGNITUDE
    //------------------------------------------------
    accel_mag=sqrtf(ax*ax+ay*ay+az*az);
    linear_mag=sqrtf(lax*lax+lay*lay+laz*laz);
    gyro_mag=sqrtf(gx*gx+gy*gy+gz*gz);

    //------------------------------------------------
    // PEAK
    //------------------------------------------------
    if(accel_mag > peak_accel + 0.2f){
        peak_accel=accel_mag;
        peak_accel_time=now;
    }

    if(gyro_mag > peak_gyro + 10){
        peak_gyro=gyro_mag;
        peak_gyro_time=now;
    }

    if(now-peak_accel_time > PEAK_HOLD){
        peak_accel=accel_mag;
        peak_accel_time=now;
    }

    if(now-peak_gyro_time > PEAK_HOLD){
        peak_gyro=gyro_mag;
        peak_gyro_time=now;
    }

    //------------------------------------------------
    // ORIENTATION
    //------------------------------------------------
    float acc_pitch =
        atan2f(-ax,sqrtf(ay*ay+az*az))*RAD2DEG;

    float acc_roll =
        atan2f(ay,az)*RAD2DEG;

    pitch =
        COMP_FILTER_ALPHA*(pitch+gx*dt)
        +(1-COMP_FILTER_ALPHA)*acc_pitch;

    roll =
        COMP_FILTER_ALPHA*(roll+gy*dt)
        +(1-COMP_FILTER_ALPHA)*acc_roll;

    //------------------------------------------------
    // STATIC DETECT
    //------------------------------------------------
    mean_lin  = 0.9f*mean_lin  + 0.1f*linear_mag;
    mean_gyro = 0.9f*mean_gyro + 0.1f*gyro_mag;

    bool acc_static = mean_lin  < STATIC_ACC_THRESHOLD;
    bool gyr_static = mean_gyro < STATIC_GYRO_THRESHOLD;

    if(acc_static && gyr_static)
        static_ms += dt_ms;
    else
        static_ms = 0;

    static_state = static_ms >= STATIC_TIME_MS;

    //------------------------------------------------
    // BASELINE
    // CHỈ SET KHI VỪA CHUYỂN SANG STATIC
    //------------------------------------------------
    if(static_state && !last_static){
        baseline_pitch=pitch;
        baseline_roll=roll;
        baseline_set=true;
    }

    last_static = static_state;

    //------------------------------------------------
    // FALL EVENT CACHE
    //------------------------------------------------
    if(peak_accel > FALL_ACC_THRESHOLD)
        impact_time = now;

    if(peak_gyro > FALL_GYRO_THRESHOLD)
        rotate_time = now;

    if(motion_get_orientation_change() >
       FALL_ORIENTATION_THRESHOLD)
        orient_time = now;
}

//====================================================

float motion_get_accel_magnitude() {
    return accel_mag;
}

float motion_get_linear_accel() {
    return linear_mag;
}

float motion_get_peak_accel() {
    return peak_accel;
}

float motion_get_stddev() {
    return mean_lin;
}

//====================================================

float motion_get_gyro_magnitude() {
    return gyro_mag;
}

float motion_get_peak_gyro() {
    return peak_gyro;
}

//====================================================

float motion_get_pitch() {
    return pitch;
}

float motion_get_roll() {
    return roll;
}

float motion_get_orientation_change() {
    if(!baseline_set) return 0;

    float dp=fabsf(pitch-baseline_pitch);
    float dr=fabsf(roll-baseline_roll);

    return dp>dr ? dp : dr;
}

//====================================================

bool motion_is_static() {
    return static_state;
}

bool motion_is_moving() {
    return linear_mag > 0.30f || gyro_mag > 25;
}

bool motion_is_free_fall() {
    return accel_mag < FREE_FALL_THRESHOLD;
}

//====================================================

bool motion_is_fall_candidate() {

    uint32_t now = millis();

    bool freeFall =
        motion_is_free_fall();

    bool impact =
        (now-impact_time) < EVENT_HOLD;

    bool rotate =
        (now-rotate_time) < EVENT_HOLD;

    bool orient =
        (now-orient_time) < EVENT_HOLD;

    return (freeFall || impact) &&
           rotate &&
           orient;
}

//====================================================

void motion_reset_baseline() {
    baseline_pitch=pitch;
    baseline_roll=roll;
    baseline_set=true;
}