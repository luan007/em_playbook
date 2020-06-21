#ifndef _GUARD_IO_H
#define _GUARD_IO_H

#include "defs.h"
#include "hal-pins.h"

SIGNAL(ENC_DELTA, "Encoder Value Changed", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_ONCE_AUTO_ZERO, 0)
SIGNAL(ENC_COUNT, "Encoder Count Changed", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_POWERLOSS, 0)           //this is handly
SIGNAL(SW_DOWN, "Timestamp of Encoder Button Pressed", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_RUNTIME, 0) //records button down
SIGNAL(SW_UP, "Timestamp Encoder Button Released", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_RUNTIME, 0)     //records button down
SIGNAL(SW_CLICK, "Clicked", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_ONCE_AUTO_ZERO, 0)                     //records button down
SIGNAL(SW_HOLD, "Long Hold Detected", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_ONCE_AUTO_ZERO, 0)           //records button down
SIGNAL(TOUCH_DOWN, "Touched", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_RUNTIME, 0)
SIGNAL(TOUCH_CLICK, "Touch Click", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_ONCE_AUTO_ZERO, 0)
SIGNAL(USER_ACTION, "Last User Interaction", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_RUNTIME, 0)

CONFIG(SW_DEBOUNCE, "Switch Debounce Time (ms)", 20, "")
CONFIG(SW_HOLD_T, "Switch Hold Duration (ms)", 3000, "")

void io_user_interaction()
{
    signal_raise(&SIG_USER_ACTION, millis(), "User Action Recorded");
}

void io_encoder_update(int A, int B)
{
    int delta = 0;
    //TODO, alg from ULP is really crude.. need fix
    if (A >= 0 && B >= 0) //this compares stuff from ULP
    {
        A = A & 0xffff;
        B = B & 0xffff;
        if ((A == 1 && B == 3) || (A == 2 && B == 0))
        {
            delta = 1;
        }
        else if ((A == 0 && B == 1) || (A == 3 && B == 2))
        {
            delta = 1;
        }
        else if ((A == 1 && B == 0) || (A == 2 && B == 3))
        {
            delta = -1;
        }
        else if ((A == 3 && B == 1) || (A == 0 && B == 2))
        {
            delta = -1;
        }
    }
    else
    {
        delta = hw_encoder.getCount();
        hw_encoder.clearCount(); //ensures we see only delta
    }
    if (delta != 0)
    {
        signal_raise(&SIG_ENC_DELTA, delta, "Encoder change detected");
        signal_raise(&SIG_ENC_COUNT, SIG_ENC_COUNT.value + delta, "Rotation change saved"); //TODO: THIS MIGHT OVERFLOW
        io_user_interaction();
    }
}

void io_touch_update()
{
    int prev_state = SIG_TOUCH_DOWN.value;
    int cur_state = 0;
    if (hw_cap_sensor.isLeftTouched() == true)
    {
        cur_state = 1;
    }
    if (hw_cap_sensor.isMiddleTouched() == true)
    {
        cur_state = 2;
    }
    if (hw_cap_sensor.isRightTouched() == true)
    {
        cur_state = 3;
    }
    if (prev_state != cur_state)
    {
        signal_raise(&SIG_TOUCH_CLICK, prev_state); //Hit!
    }
    signal_raise(&SIG_TOUCH_DOWN, cur_state);
    if (cur_state != 0 || SIG_TOUCH_CLICK.value > 0)
    {
        io_user_interaction();
    }
}

int _hold_flag = 0;
void io_sw_update()
{
    int sw_state = digitalRead(SW);
    if (SIG_SW_DOWN.value == 0 && sw_state == 0)
    {
        //pressed, record
        signal_raise(&SIG_SW_DOWN, millis()); //trigger
    }
    if (sw_state == 1 && SIG_SW_DOWN.value > 0)
    {
        int _press_start = SIG_SW_DOWN.value;
        signal_raise(&SIG_SW_DOWN, 0);        //release trigger
        signal_raise(&SIG_SW_UP, millis()); //duration
        if ((millis() - _press_start) > CFG_SW_DEBOUNCE.value64)
        {
            signal_raise(&SIG_SW_CLICK, 1);
        }
        _hold_flag = 0; //reset hold detection
    }

    if (sw_state == 0 && _hold_flag == 0)
    {
        if ((millis() - SIG_SW_DOWN.value) > CFG_SW_HOLD_T.value64)
        {
            _hold_flag = 1;
            signal_raise(&SIG_SW_HOLD, 1);
        }
    }
    if (sw_state == 0)
    {
        io_user_interaction();
    }
}

void hal_io_setup()
{
    signal_register(&SIG_ENC_DELTA);
    signal_register(&SIG_ENC_COUNT);
    signal_register(&SIG_SW_DOWN);
    signal_register(&SIG_SW_UP);
    signal_register(&SIG_SW_CLICK);
    signal_register(&SIG_SW_HOLD);
    signal_register(&SIG_TOUCH_DOWN);
    signal_register(&SIG_TOUCH_CLICK);
    signal_register(&SIG_USER_ACTION);
    config_register(&CFG_SW_DEBOUNCE);
    config_register(&CFG_SW_HOLD_T);

    Wire.begin();
    hw_cap_sensor.begin();
    hw_cap_sensor.setSensitivity(SENSITIVITY_128X);

    pinMode(SW, INPUT);
    pinMode(DT, INPUT);
    pinMode(CLK, INPUT);
    pinMode(CAP_ALERT, INPUT);

    hw_encoder.attachHalfQuad(DT, CLK);
}

void hal_io_loop()
{
    io_touch_update();
    io_sw_update();
    io_encoder_update(-1, -1);
}

#endif