#ifndef _GUARD_IO_H
#define _GUARD_IO_H

#include <ESP32Encoder.h>
#include <SparkFun_CAP1203_Registers.h>
#include <SparkFun_CAP1203_Types.h>
#include <Wire.h>
#include "def.h"
#include "hal-pins.h"



ESP32Encoder encoder;
CAP1203 sensor;


void io_user_interaction()
{
}

int prev_count = 0;
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
        if (prev_count != dir)
        {
            prev_count = dir;
        }
        // Serial.println(String("count:") + dir);
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

void hal_io_setup()
{
    Serial.begin(115200);
    Wire.begin();
    sensor.begin();
    sensor.setSensitivity(SENSITIVITY_128X);

    pinMode(SW, INPUT);
    pinMode(DT, INPUT);
    pinMode(CLK, INPUT);
    pinMode(CAP_ALERT, INPUT);

    encoder.attachHalfQuad(DT, CLK);
}

void hal_io_loop()
{
    // _sleep();
    // if (SIG_ENCODER_DELTA != 0)
    // {
    //     Serial.println("SIG_ENCODER_DELTA");
    //     Serial.println(SIG_ENCODER_DELTA);
    // }
    // if (SIG_TOUCH_CLICK != 0)
    // {
    //     Serial.println("SIG_TOUCH_CLICK");
    //     Serial.println(SIG_TOUCH_CLICK);
    // }
    // if (SIG_ENCODER_CLICK != 0)
    // {
    //     Serial.println("SIG_ENCODER_CLICK");
    //     Serial.println(SIG_ENCODER_CLICK);
    // }
    // if (SIG_ENCODER_HOLD != 0)
    // {
    //     Serial.println("SIG_ENCODER_HOLD");
    //     Serial.println(SIG_ENCODER_HOLD);
    // }
    io_touch_update();
    io_press_button_update();
    io_encoder_update(-1, -1);
}

#endif