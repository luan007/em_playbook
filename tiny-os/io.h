#ifndef _GUARD_IO_H
#define _GUARD_IO_H

#include <ESP32Encoder.h>
#include <SparkFun_CAP1203_Registers.h>
#include <SparkFun_CAP1203_Types.h>
#include <Wire.h>

#include "FS.h"
#include "SPIFFS.h"
#include "FFat.h"
#define USE_FS FFat

#define LED1 32
#define LED2 33

#define SW 27
#define SW_GPIO GPIO_NUM_27
#define DT 13
#define DT_GPIO GPIO_NUM_13
#define CLK 14
#define CLK_GPIO GPIO_NUM_14

#define CAP_ALERT 26
#define CAP_ALERT_GPIO GPIO_NUM_26
#define CAP_SDA 21
#define CAP_SCL 22

#define PWR_CONTROL 17
#define TPS_CONTROL 15
#define IT8951_RESET 16

ESP32Encoder encoder;
CAP1203 sensor;

#define SIGCLR(sig) sig = 0;
#define SIGSET(sig, val) sig = val;
#define SIGNAL long

long INTERACTION_TIMESTAMP = 0;

SIGNAL SIG_WAKE = 0;
SIGNAL SIG_USER_INTERACTION = 0;
SIGNAL SIG_WAKE_BY_INPUT = 0;
SIGNAL SIG_WAKE_BY_TIMER = 0;
SIGNAL SIG_GOING_SLEEP = 0;
SIGNAL SIG_ENCODER_DELTA = 0;
SIGNAL SIG_TOUCH_CLICK = 0;
SIGNAL SIG_TOUCH = 0;
SIGNAL SIG_ENCODER_CLICK = 0;
SIGNAL SIG_ENCODER_PRESS = 0;
SIGNAL SIG_ENCODER_HOLD = 0;
int __SIG_ENCODER_HOLD__triggered = 0; //private sig

void io_user_interaction()
{
    INTERACTION_TIMESTAMP = millis(); //sanity
}

void io_encoder_update(int A, int B)
{
    SIG_ENCODER_DELTA = 0;
    if (A >= 0 && B >= 0)
    {
        A = A & 0xffff;
        B = B & 0xffff;
        if ((A == 1 && B == 3) || (A == 2 && B == 0))
        {
            SIG_ENCODER_DELTA = 1;
        }
        else if ((A == 0 && B == 1) || (A == 3 && B == 2))
        {
            SIG_ENCODER_DELTA = 1;
        }
        else if ((A == 1 && B == 0) || (A == 2 && B == 3))
        {
            SIG_ENCODER_DELTA = -1;
        }
        else if ((A == 3 && B == 1) || (A == 0 && B == 2))
        {
            SIG_ENCODER_DELTA = -1;
        }
    }
    else
    {
        int dir = encoder.getCount();
        SIG_ENCODER_DELTA = dir;
        SIG_ENCODER_DELTA = SIG_ENCODER_DELTA > 0 ? 1 : (SIG_ENCODER_DELTA < 0 ? -1 : SIG_ENCODER_DELTA);
        encoder.clearCount();
    }
    if (SIG_ENCODER_DELTA != 0)
    {
        io_user_interaction();
    }
}

void io_touch_update()
{
    int _prev_touch = SIG_TOUCH;
    SIG_TOUCH_CLICK = 0;
    SIG_TOUCH = 0;

    if (sensor.isLeftTouched() == true)
    {
        SIG_TOUCH = 1;
    }
    if (sensor.isMiddleTouched() == true)
    {
        SIG_TOUCH = 2;
    }
    if (sensor.isRightTouched() == true)
    {
        SIG_TOUCH = 3;
    }
    if (_prev_touch != SIG_TOUCH)
    {
        SIG_TOUCH_CLICK = _prev_touch;
    }
    if (SIG_TOUCH != 0 || SIG_TOUCH_CLICK > 0)
    {
        io_user_interaction();
    }
}

void io_press_button_update()
{
    SIG_ENCODER_CLICK = 0;
    SIG_ENCODER_HOLD = 0;
    if (SIG_ENCODER_PRESS == 0 && digitalRead(SW) == 0)
    {
        SIG_ENCODER_PRESS = millis();
        __SIG_ENCODER_HOLD__triggered = 0;
    }
    if (digitalRead(SW) == 1)
    {
        if (SIG_ENCODER_PRESS > 0 && (millis() - SIG_ENCODER_PRESS) > 10)
        {
            SIG_ENCODER_CLICK = 1;
        }
        SIG_ENCODER_PRESS = 0;
    }

    if (__SIG_ENCODER_HOLD__triggered == 0 && SIG_ENCODER_PRESS > 0 && (millis() - SIG_ENCODER_PRESS) > 2000)
    {
        SIG_ENCODER_HOLD = 1;
        __SIG_ENCODER_HOLD__triggered = 1;
    }

    if (SIG_ENCODER_PRESS > 0)
    {
        io_user_interaction();
    }
}

void setup_fs()
{
    // USE_FS.format();
    USE_FS.begin();
}

void setup_serial()
{
    Serial.begin(115200);
}

void setup_io()
{
    setup_serial();
    setup_fs();
    Wire.begin();
    sensor.begin();
    sensor.setSensitivity(SENSITIVITY_128X);
    sensor.setInterruptEnabled();

    // pinMode(LED1, OUTPUT);
    // pinMode(LED2, OUTPUT);
    // digitalWrite(LED1, HIGH);
    // digitalWrite(LED2, HIGH);
    pinMode(SW, INPUT);
    pinMode(DT, INPUT);
    pinMode(CLK, INPUT);
    pinMode(CAP_ALERT, INPUT);
    // attachInterrupt(SW, swInterrupt, FALLING);
    // attachInterrupt(CAP_ALERT, capInterrupt, FALLING);
    // attachInterrupt(DT, encoderInterrupt, FALLING);
    // attachInterrupt(CLK, encoderInterrupt, FALLING);
    encoder.attachHalfQuad(CLK, DT);
}

void io_loop()
{
    // _sleep();
    if (SIG_ENCODER_DELTA != 0)
    {
        Serial.println("SIG_ENCODER_DELTA");
        Serial.println(SIG_ENCODER_DELTA);
    }
    if (SIG_TOUCH_CLICK != 0)
    {
        Serial.println("SIG_TOUCH_CLICK");
        Serial.println(SIG_TOUCH_CLICK);
    }
    if (SIG_ENCODER_CLICK != 0)
    {
        Serial.println("SIG_ENCODER_CLICK");
        Serial.println(SIG_ENCODER_CLICK);
    }
    if (SIG_ENCODER_HOLD != 0)
    {
        Serial.println("SIG_ENCODER_HOLD");
        Serial.println(SIG_ENCODER_HOLD);
    }
    io_encoder_update(-1, -1);
    io_touch_update();
    io_press_button_update();
}

#endif