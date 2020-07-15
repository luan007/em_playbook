#ifndef _GUARD_IO_H
#define _GUARD_IO_H

///////////////////////// PINS

#define LED1 32
#define LED2 33

#define BAT_IN 34
#define VBUS_IN 35

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

///////////////////////// LIBS

#include "shared.h"
#include <ESP32Encoder.h>
#include <SparkFun_CAP1203_Registers.h>
#include <SparkFun_CAP1203_Types.h>
#include <Wire.h>
#include <RTClib.h>
RTC_DS1307 rtc;
///////////////////////// OBJS

ESP32Encoder hw_encoder;
CAP1203 hw_cap_sensor;

///////////////////////// SIGNALS

SIGNAL(ENC_DELTA, SIG_ALL, SIG_ONCE, 0)
SIGNAL(ENC_COUNT, SIG_ALL, SIG_POWERLOSS, 0)
SIGNAL(SW_PRESSING, SIG_ALL, SIG_RUNTIME, 0)
SIGNAL(SW_DOWN, SIG_ALL, SIG_RUNTIME, 0)
SIGNAL(SW_UP, SIG_ALL, SIG_RUNTIME, 0)
SIGNAL(SW_CLICK, SIG_ALL, SIG_ONCE, 0)
SIGNAL(SW_HOLD, SIG_ALL, SIG_ONCE, 0)
SIGNAL(TOUCH_DOWN, SIG_ALL, SIG_RUNTIME, 0)
SIGNAL(TOUCH_CLICK, SIG_ALL, SIG_ONCE, 0)
SIGNAL(USER_ACTION, SIG_NONE, SIG_RUNTIME, 0)
SIGNAL(RTC_INVALID, SIG_NONE, SIG_RUNTIME, 0)

void io_user_interaction(const char *reason)
{
    // DEBUG("USER ACTION", reason);
    sig_set(&SIG_USER_ACTION, millis());
}

void io_encoder_update(int A, int B)
{
    if (SIG_ENC_DELTA.value != 0)
    {
        sig_clear(&SIG_ENC_DELTA, 0);
    }
    int delta = 0;
    bool cannot_determine = false;
    //TODO, alg from ULP is really crude.. need fix
    if (A >= 0 && B >= 0) //this compares stuff from ULP
    {
        A = A & 0xffff;
        B = B & 0xffff;
        // Serial.println(String(A) + ":" + String(B));
        if ((A == 1 && B == 3) || (A == 2 && B == 0))
        {
            delta = -1;
        }
        else if ((A == 0 && B == 1) || (A == 3 && B == 2))
        {
            delta = -1;
        }
        else if ((A == 1 && B == 0) || (A == 2 && B == 3))
        {
            delta = 1;
        }
        else if ((A == 3 && B == 1) || (A == 0 && B == 2))
        {
            delta = 1;
        }
        else if ((A == 3 && B == 0) || (A == 0 && B == 3))
        {
            cannot_determine = true;
        }
    }
    else
    {
        delta = hw_encoder.getCount();
        hw_encoder.clearCount();
    }
    if (delta != 0 || cannot_determine)
    {
        sig_set(&SIG_ENC_DELTA, delta);
        sig_set(&SIG_ENC_COUNT, SIG_ENC_COUNT.value + delta);
        io_user_interaction("ENCODER");
    }
}

void io_touch_update()
{
    if (SIG_TOUCH_CLICK.value > 0)
    {
        sig_clear(&SIG_TOUCH_CLICK, 0);
    }
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
        sig_set(&SIG_TOUCH_CLICK, prev_state); //Hit!
    }
    sig_set(&SIG_TOUCH_DOWN, cur_state);
    if (cur_state != 0)
    {
        io_user_interaction("TOUCH");
    }
}

int _hold_flag = 0;
#define SW_CLICK_LEN 50
#define SW_HOLD_T 5000
void io_sw_update()
{
    int sw_state = digitalRead(SW);
    sig_set(&SIG_SW_PRESSING, sw_state == 1 ? 0 : 1);

    if (SIG_SW_CLICK.value)
    {
        sig_clear(&SIG_SW_CLICK, 0); //Hit!
    }
    if (SIG_SW_HOLD.value)
    {
        sig_clear(&SIG_SW_HOLD, 0); //Hit!
    }
    if (SIG_SW_DOWN.value == 0 && sw_state == 0)
    {
        sig_set(&SIG_SW_DOWN, millis());
    }
    if (sw_state == 1 && SIG_SW_DOWN.value > 0)
    {
        int _press_start = SIG_SW_DOWN.value;
        sig_clear(&SIG_SW_DOWN, 0);
        sig_set(&SIG_SW_UP, millis() - _press_start); //duration
        if (SIG_SW_UP.value > SW_CLICK_LEN)
        {
            sig_set(&SIG_SW_CLICK, 1);
        }
        _hold_flag = 0; //reset hold detection
    }
    if (sw_state == 0)
    {
        io_user_interaction("SW");
        if (_hold_flag == 0 && (millis() - SIG_SW_DOWN.value) > SW_HOLD_T)
        {
            _hold_flag = 1;
            sig_set(&SIG_SW_HOLD, 1);
        }
    }
}

#define LED_A_ON digitalWrite(LED1, HIGH)
#define LED_A_OFF digitalWrite(LED1, LOW)
#define LED_B_ON digitalWrite(LED2, HIGH)
#define LED_B_OFF digitalWrite(LED2, LOW)

bool hal_read_ota_mode_entry() {
    pinMode(SW, INPUT);
    return !digitalRead(SW);
}

void hal_io_setup()
{
    Wire.begin();
    rtc.begin();

    if (!rtc.isrunning())
    {
        sig_set(&SIG_RTC_INVALID, 1);
    }

    hw_cap_sensor.begin();
    hw_cap_sensor.setSensitivity(SENSITIVITY_128X);

    pinMode(SW, INPUT);
    pinMode(DT, INPUT);
    pinMode(CLK, INPUT);
    pinMode(CAP_ALERT, INPUT);
    pinMode(BAT_IN, INPUT);
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);

    hw_encoder.attachHalfQuad(DT, CLK);
    int r = max(0, min(100, (((analogRead(BAT_IN) * 1650 / 1000) / 3) - 1000) * 198 / 1000));
    sig_set(&SIG_BAT, r);
    Serial.print("BATTERY %%% ");
    Serial.print(r);
    Serial.println(" %%% ");
}

uint32_t rtc_unix_time()
{
    DateTime now = rtc.now();
    return (uint32_t)now.unixtime();
}

DateTime rtc_date_time()
{
    DateTime now = rtc.now();
    return now;
}

void hal_io_loop()
{
    io_touch_update();
    io_sw_update();
    io_encoder_update(-1, -1);
    // wdt_clear();
}

#endif