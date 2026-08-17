#ifndef FIREBASE_H
#define FIREBASE_H

#include <Arduino.h>


//======================================
// DATA PACKET
//======================================

struct FirebasePacket
{
    int bpm;
    int spo2;

    bool fallState;

    String datetime;

    String zone;       
    String wearing;    

    struct
    {
        float lat;
        float lng;
    } gps;

};

//======================================
// API
//======================================

void firebase_init();

bool firebase_is_connected();

bool firebase_pushRealtime(
    const FirebasePacket &data);

bool firebase_pushHistory(
    const FirebasePacket &data);

void firebase_update(
    const FirebasePacket &data);

enum FirebaseEvent
{
    FB_NONE,

    FB_WARNING_ENTER,
    FB_DANGER_ENTER,

    FB_FALL_DETECTED,
    FB_FALL_CANCEL,

    FB_SOS_SENT,

    FB_WARNING_SMS,
    FB_DANGER_SMS,

    FB_RECOVER
};

#endif