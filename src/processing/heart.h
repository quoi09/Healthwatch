// src/processing/heart.h
#ifndef HEART_H
#define HEART_H

#include <Arduino.h>

// Initialize heart rate processing
void heart_init();

// Process raw IR/RED data from MAX30102
// ir_value   : raw IR LED value
// red_value  : raw RED LED value
// wearing    : whether sensor is being worn
// now_ms     : current timestamp
void heart_process(uint32_t ir_value, uint32_t red_value, bool wearing, unsigned long now_ms);

// Get current BPM (beats per minute)
float heart_get_bpm();

float heart_get_spo2();
bool heart_spo2_valid();

// Check if sensor is being worn
bool heart_is_wearing();

// Check if HR values are stable/valid
bool heart_is_valid();

// Get raw IR/RED buffers for debugging
void heart_get_raw_values(uint32_t &ir_last, uint32_t &red_last);

uint16_t heart_get_rr_interval();

#endif // HEART_H
