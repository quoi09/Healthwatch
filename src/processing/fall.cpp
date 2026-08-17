//====================================================
// fall.cpp
//====================================================
#include "fall.h"
#include "motion.h"
#include <Arduino.h>

//====================================================

static const uint8_t VERIFY_SCORE_MAX = 3;
static const uint32_t VERIFY_TIMEOUT_MS = 8000;
static const uint32_t STATIC_CONFIRM_MS = 1500;
static const uint32_t MOVE_CANCEL_MS = 3000;

//====================================================

static FallState state = FALL_IDLE;

static bool detected = false;
static bool confirmed = false;

static float probability = 0;
static uint8_t score = 0;

static uint32_t t_verify = 0;
static uint32_t t_static = 0;
static uint32_t t_cancel = 0;
static uint32_t t_move = 0;

//====================================================

void fall_reset() {
    state = FALL_IDLE;

    detected = false;
    confirmed = false;

    probability = 0;
    score = 0;

    t_verify = 0;
    t_static = 0;
    t_cancel = 0;
    t_move = 0;
}

//====================================================

void fall_init() {
    fall_reset();
}

//====================================================

void fall_cancel() {
    state = FALL_CANCELLED;

    detected = false;
    confirmed = false;

    probability = 0;
    score = 0;

    t_cancel = millis();
}

//====================================================

static void update_score(bool candidate) {

    if(candidate) {

        if(score < VERIFY_SCORE_MAX)
            score++;

    } else {

        if(score > 0)
            score--;
    }
}

//====================================================

FallState fall_update(bool fallCandidate) {

    uint32_t now = millis();

    //------------------------------------------------
    // CANCEL
    //------------------------------------------------
    if(state == FALL_CANCELLED) {

        if(now - t_cancel > 3000)
            fall_reset();

        return state;
    }

    //------------------------------------------------
    // CONFIRMED
    //------------------------------------------------
    if(state == FALL_CONFIRMED)
        return state;

    //------------------------------------------------
    // SCORE
    //------------------------------------------------
    update_score(fallCandidate);

    probability =
        (float)score /
        (float)VERIFY_SCORE_MAX;

    //------------------------------------------------
    switch(state) {

        //--------------------------------------------
        case FALL_IDLE:

            /*
             * Chỉ cần đạt 2/3 điểm là vào VERIFY
             * Không cần 3 hit liên tục 
             */

            if(score >= 2) {

                state = FALL_VERIFY;

                detected = true;
                confirmed = false;

                t_verify = now;
                t_static = 0;
                t_move = 0;

                Serial.println("[FALL] VERIFY");
            }

            break;

        //--------------------------------------------
        case FALL_VERIFY:

            //----------------------------------------
            // timeout
            //----------------------------------------
            if(now - t_verify > VERIFY_TIMEOUT_MS) {

                Serial.println("[FALL] TIMEOUT");

                fall_reset();
                break;
            }

            //----------------------------------------
            // movement cancel
            //----------------------------------------
            if(motion_is_moving()) {

                if(t_move == 0)
                    t_move = now;

                /*
                 * Chỉ hủy nếu chuyển động mạnh
                 * kéo dài liên tục >3s
                 */

                if(now - t_move > MOVE_CANCEL_MS) {

                    Serial.println("[FALL] MOVING CANCEL");

                    fall_reset();
                }

            } else {

                t_move = 0;
            }

            //----------------------------------------
            // static confirm
            //----------------------------------------
            if(motion_is_static()) {

                if(t_static == 0)
                    t_static = now;

                if(now - t_static >= STATIC_CONFIRM_MS) {

                    state = FALL_CONFIRMED;

                    detected = false;
                    confirmed = true;

                    probability = 1.0f;

                    Serial.println("[FALL] CONFIRMED");
                }

            } else {

                t_static = 0;
            }

            break;

        //--------------------------------------------
        default:
            break;
    }

    return state;
}

//====================================================

FallState fall_get_state() {
    return state;
}

//====================================================

bool fall_detected() {
    return detected;
}

//====================================================

bool fall_is_confirmed() {
    return confirmed;
}

//====================================================

float fall_probability() {
    return probability;
}