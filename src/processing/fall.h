#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {

FALL_IDLE=0,

FALL_VERIFY,

FALL_CONFIRMED,

FALL_CANCELLED

}
FallState;


// init

void fall_init();

void fall_reset();

void fall_cancel();


// update

FallState fall_update(

bool fallCandidate

);


// getters

FallState fall_get_state();

bool fall_detected();

bool fall_is_confirmed();

float fall_probability();