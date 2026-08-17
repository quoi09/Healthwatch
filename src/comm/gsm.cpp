//====================================================
// gsm.cpp
//====================================================
#include "gsm.h"
#include <Arduino.h>
#include <config.h>

//====================================================
// SERIAL
//====================================================
static HardwareSerial gsmSerial(1);

//====================================================
// STATE
//====================================================
static bool gsm_ready = false;
static bool gsm_sleeping = false;
static bool g_callActive = false;

//====================================================
// INIT FSM
//====================================================
static bool initStarted = false;
static bool initDone = false;
static uint32_t initStartMs = 0;
static uint32_t lastInitTry = 0;

//====================================================
// CONFIG
//====================================================
static const uint32_t GSM_BOOT_TIME_MS = 8000;
static const uint32_t GSM_INIT_RETRY_MS = 2000;

//====================================================
// INTERNAL WAIT RESPONSE
//====================================================
static bool gsm_waitResponse(const char* expect, uint32_t timeout)
{
    String resp = "";
    uint32_t start = millis();

    while (millis() - start < timeout)
    {
        while (gsmSerial.available())
        {
            char c = gsmSerial.read();
            resp += c;

            //------------------------------------------------
            // DEBUG
            //------------------------------------------------
            Serial.write(c);

            //------------------------------------------------
            // ASYNC EVENTS
            //------------------------------------------------
            if (resp.indexOf("NO CARRIER") >= 0) { g_callActive = false; Serial.println("[GSM] CALL ENDED"); }
            if (resp.indexOf("BUSY") >= 0)       { g_callActive = false; Serial.println("[GSM] BUSY"); }
            if (resp.indexOf("NO ANSWER") >= 0)  { g_callActive = false; Serial.println("[GSM] NO ANSWER"); }

            //------------------------------------------------
            // EXPECT
            //------------------------------------------------
            if (resp.indexOf(expect) >= 0)
            {
                return true;
            }
        }
        yield();
        delay(1);
    }
    return false;
}

//====================================================
// SEND AT
//====================================================
bool gsm_sendAT(const char* cmd, const char* expect, uint32_t timeout)
{
    gsmSerial.println(cmd);
    Serial.print("[GSM] >> "); Serial.println(cmd);

    bool ok = gsm_waitResponse(expect, timeout);

    Serial.print("[GSM] "); Serial.println(ok ? "OK" : "FAIL");
    return ok;
}

//====================================================
// INIT
//====================================================
void gsm_init()
{
    //------------------------------------------------
    // DTR
    //------------------------------------------------
    pinMode(GSM_DTR_PIN, OUTPUT);
    digitalWrite(GSM_DTR_PIN, LOW);

    //------------------------------------------------
    // UART
    //------------------------------------------------
    gsmSerial.begin(GSM_BAUDRATE, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);

    //------------------------------------------------
    // STATE
    //------------------------------------------------
    gsm_ready = false; gsm_sleeping = false; g_callActive = false;

    //------------------------------------------------
    // INIT FSM
    //------------------------------------------------
    initStarted = true; initDone = false;
    initStartMs = millis(); lastInitTry = 0;

    Serial.println("[GSM] BOOTING...");
}

//====================================================
// UPDATE
//====================================================
void gsm_update()
{
    //------------------------------------------------
    // BOOT WAIT
    //------------------------------------------------
    if (!initDone)
    {
        if (millis() - initStartMs < GSM_BOOT_TIME_MS)
        {
            return;
        }

        //------------------------------------------------
        // RETRY TIMER
        //------------------------------------------------
        if (millis() - lastInitTry >= GSM_INIT_RETRY_MS)
        {
            lastInitTry = millis();

            //------------------------------------------------
            // BASIC AT
            //------------------------------------------------
            if (gsm_sendAT("AT", "OK", 2000)) // KIEM TRA KET NOI
            {
                gsm_sendAT("ATE0", "OK", 1000);
                gsm_sendAT("AT+CMGF=1", "OK", 1000);

                gsm_ready = true; initDone = true;
                Serial.println("[GSM] READY");
            }
        }
    }

    //------------------------------------------------
    // ASYNC PARSER
    //------------------------------------------------
    static String line = "";

    while (gsmSerial.available())
    {
        char c = gsmSerial.read();
        Serial.write(c);

        //------------------------------------------------
        // LINE END
        //------------------------------------------------
        if (c == '\n' || c == '\r')
        {
            line.trim();

            //------------------------------------------------
            // CALL END / STATUS PARSING
            //------------------------------------------------
            if (line.indexOf("NO CARRIER") >= 0) { g_callActive = false; Serial.println("[GSM] CALL ENDED"); }
            if (line.indexOf("BUSY") >= 0)       { g_callActive = false; Serial.println("[GSM] BUSY"); }
            if (line.indexOf("NO ANSWER") >= 0)  { g_callActive = false; Serial.println("[GSM] NO ANSWER"); }

            line = "";
        }
        else
        {
            line += c;
        }
    }
}

//====================================================
// STATUS
//====================================================
bool gsm_is_ready()       { return gsm_ready; }
bool gsm_isSleeping()     { return gsm_sleeping; }
bool gsm_isCallActive()   { return g_callActive; }

//====================================================
// WAKEUP
//====================================================
void gsm_wakeup()
{
    if (!gsm_ready)    { return; }
    if (!gsm_sleeping) { return; }

    Serial.println("[GSM] WAKE");

    //------------------------------------------------
    // DTR LOW & TEST UART
    //------------------------------------------------
    digitalWrite(GSM_DTR_PIN, LOW);
    delay(100);

    gsmSerial.println("AT");
    if (gsm_waitResponse("OK", 3000))
    {
        gsm_sleeping = false;
        Serial.println("[GSM] WAKE OK");
    }
    else
    {
        gsm_sleeping = false;
        Serial.println("[GSM] WAKE TIMEOUT");
    }
}

//====================================================
// SLEEP
//====================================================
void gsm_sleep()
{
    if (!gsm_ready)    { return; }
    if (gsm_sleeping)  { return; }
    if (g_callActive)  { return; }

    Serial.println("[GSM] SLEEP");

    //------------------------------------------------
    // LIGHT SLEEP & DTR HIGH
    //------------------------------------------------
    gsm_sendAT("AT+CSCLK=1", "OK", 1000);
    gsmSerial.flush();

    digitalWrite(GSM_DTR_PIN, HIGH);
    gsm_sleeping = true;
}

//====================================================
// SMS
//====================================================
bool gsm_sendSMS(const char* phone, const char* msg)
{
    if (!gsm_ready) { return false; }
    if (gsm_sleeping) { gsm_wakeup(); }

    // CLEAR UART
    while (gsmSerial.available()) { gsmSerial.read(); }

    // SMS MODE
    gsmSerial.print("AT+CMGS=\""); gsmSerial.print(phone); gsmSerial.println("\"");

    // WAIT >
    if (!gsm_waitResponse(">", 5000))
    {
        Serial.println("[GSM] NO PROMPT");
        return false;
    }

    // SEND BODY
    gsmSerial.print(msg); gsmSerial.write(26);

    // WAIT RESULT
    bool ok = gsm_waitResponse("+CMGS:", 15000);
    Serial.print("[GSM] SMS "); Serial.println(ok ? "OK" : "FAIL");

    return ok;

    
}

//====================================================
// CALL
//====================================================
bool gsm_call(const char* phone)
{
    if (!gsm_ready) { return false; }
    if (gsm_sleeping) { gsm_wakeup(); }

    // CLEAR UART
    while (gsmSerial.available()) { gsmSerial.read(); }

    // CALL
    gsmSerial.print("ATD"); gsmSerial.print(phone); gsmSerial.println(";");

    String resp;
    uint32_t start = millis();

    while (millis() - start < 5000)
    {
        while (gsmSerial.available())
        {
            char c = gsmSerial.read();
            resp += c; Serial.write(c);

            // SUCCESS
            if (resp.indexOf("OK") >= 0 || resp.indexOf("CONNECT") >= 0)
            {
                g_callActive = true; Serial.println("[GSM] CALL START");
                return true;
            }

            // FAIL
            if (resp.indexOf("BUSY") >= 0 || resp.indexOf("NO CARRIER") >= 0 || resp.indexOf("NO ANSWER") >= 0)
            {
                g_callActive = false; Serial.println("[GSM] CALL FAIL");
                return false;
            }
        }
        yield();
        delay(1);
    }

    Serial.println("[GSM] CALL TIMEOUT");
    return false;
}

//====================================================
// HANGUP
//====================================================
bool gsm_hangup()
{
    if (!gsm_ready) { return false; }

    gsmSerial.println("ATH");
    bool ok = gsm_waitResponse("OK", 3000);

    g_callActive = false; // FORCE IDLE

    // MODEM RECOVERY
    gsm_sendAT("AT", "OK", 3000);
    delay(2000);

    Serial.print("[GSM] HANGUP "); Serial.println(ok ? "OK" : "FAIL");
    return ok;
}

//====================================================
// EMERGENCY SMS
//====================================================
bool gsm_sendEmergency(const char* phone, float lat, float lng, const char* msg)
{
    char buffer[256];

    if (lat != 0 && lng != 0)
    {
        // Lưu ý: Chuỗi format gốc của bạn thiếu các định dạng %f cho lat và lng. 
        // Đã sửa lại định dạng để truyền đúng tọa độ vào link Google Maps.
        snprintf(buffer, sizeof(buffer), "%s\n[https://maps.google.com/?q=%f,%f]", msg, lat, lng);
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "%s\nGPS unavailable", msg);
    }

    return gsm_sendSMS(phone, buffer);
}