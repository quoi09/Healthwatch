#include "max30102.h"
#include "config.h"
#include <Wire.h>
#include "MAX30105.h"
#include "processing/heart.h"

static MAX30105 sensor;
static bool sensorReady = false;

// Chỉ giữ 1 ngưỡng duy nhất
#define FINGER_THRESHOLD 20000

void max_init()
{
    if(!sensor.begin(Wire, I2C_SPEED_FAST))
    {
        Serial.println("MAX30102 not found");
        sensorReady = false;
        return;
    }

    //====================================================
    // CẤU HÌNH TỐI ƯU ĐO NHỊP TIM NHANH + SpO2
    //====================================================
byte ledBrightness = 60;   // GIẢM MẠNH: 80→60, tránh saturation 262143
    byte sampleAverage = 4;    // Giảm từ 8→4: giữ lại AC pulse peak
    byte ledMode       = 2;    // RED + IR cho SpO2
    int  sampleRate    = 400;  // Tăng lên 400: effective rate = 400/4 = 100Hz
    int  pulseWidth    = 215;  // Giảm từ 411→215: giảm tích lũy photon → tránh sat
    int  adcRange      = 16384;

    sensor.setup(
        ledBrightness,
        sampleAverage,
        ledMode,
        sampleRate,
        pulseWidth,
        adcRange
    );



    sensorReady = true;

    Serial.println("MAX30102 ready");
}

bool max_read(
    uint32_t &ir,
    uint32_t &red,
    bool &wearing
)
{
    if(!sensorReady)
        return false;

    sensor.check();

    int samples = 0;

   while(sensor.available())
{
    ir  = sensor.getIR();
    red = sensor.getRed();

    wearing = (ir > FINGER_THRESHOLD);

    heart_process(
        ir,
        red,
        wearing,
        millis()
    );

    sensor.nextSample();

    samples++;
}

    if(samples == 0)
        return false;

    static uint32_t dbg = 0;
    if(millis() - dbg > 1000)
    {
        dbg = millis();
        Serial.printf("[MAX] FIFO=%d IR=%lu RED=%lu\n",
                      samples, ir, red);
    }

    wearing = (ir > FINGER_THRESHOLD);

    return true;
}

bool max_is_ready()
{
    return sensorReady;
}

//----------------------------------------------------
// POWER SAVE
//----------------------------------------------------

void max_sleep()
{
    if(!sensorReady)
        return;

    sensor.shutDown();

    Serial.println("[MAX] SLEEP");
}

void max_wakeup()
{
    if(!sensorReady)
        return;

    sensor.wakeUp();

    delay(10);

    Serial.println("[MAX] WAKE");
}

void max_process_fifo(MaxSampleCallback cb)
{
    if(!sensorReady || cb == nullptr)
        return;

    sensor.check();

    while(sensor.available())
    {
        uint32_t ir  = sensor.getIR();
        uint32_t red = sensor.getRed();

        cb(
            ir,
            red,
            ir > FINGER_THRESHOLD
        );

        sensor.nextSample();
    }
}