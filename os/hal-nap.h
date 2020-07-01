#ifndef _GUARD_H_NAP
#define _GUARD_H_NAP

#include "esp32/ulp.h"
#include "ulp_main.h"
#include "ulptool.h"
#include "driver/rtc_io.h"
#include "driver/gpio.h"
#include "shared.h"
#include "hal-io.h"

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");

//this is not recommended to use
uint32_t next_wake_interval = -1;
void nap_set_sleep_duration(uint32_t override_next_wake)
{
    next_wake_interval = override_next_wake;
}

//kickstart ULP
void nap_enter_sleep()
{
    LED_A_OFF;
    LED_B_OFF;
    sig_save(true); //force save

    rtc_gpio_init(GPIO_NUM_26);
    rtc_gpio_set_direction(GPIO_NUM_26, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_init(GPIO_NUM_27);
    rtc_gpio_set_direction(GPIO_NUM_27, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_init(GPIO_NUM_13);
    rtc_gpio_set_direction(GPIO_NUM_13, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_init(GPIO_NUM_14);
    rtc_gpio_set_direction(GPIO_NUM_14, RTC_GPIO_MODE_INPUT_ONLY);

    esp_err_t err = ulptool_load_binary(0, ulp_main_bin_start, (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));

    esp_sleep_enable_timer_wakeup((uint64_t)(SIG_WAKE_AFTER.value) * 1000ULL);
    // nap_schedule_next_wake();
    esp_sleep_enable_ulp_wakeup();
    ulp_set_wakeup_period(0, 50); // needs to be fast in order to get correct result
    err = ulp_run((&ulp_entry - RTC_SLOW_MEM) / sizeof(uint32_t));
    if (err)
        Serial.println("Error Starting ULP Coprocessor");

    Serial.println("Entering Sleep Now.");

    ulp__switch = 0;
    ulp__touch = 0;
    ulp__encoder_state = 0;
    ulp__prev_encoder_state = 255;
    esp_deep_sleep_start();
}

uint32_t next_sleep = 0; //immediate, as soon as possible
void nap_set_enter_sleep_after(int abs_ms, bool override = false)
{
    next_sleep = next_sleep > abs_ms ? next_sleep : abs_ms;
    if (override)
    {
        next_sleep = abs_ms;
    }
}

void nap_try_sleep(bool NOW, uint32_t set_schedule = 0)
{
    if (NOW || millis() > next_sleep)
    {
        uint32_t ms = (uint32_t)schedule_compute_millis();
        ms = set_schedule == 0 ? ms : set_schedule;
        if (next_wake_interval != -1)
        {
            ms = next_wake_interval;
            DEBUG("SLEEP", (String("OVERRIDE = ") + ms).c_str());
        }
        else
        {
            DEBUG("SLEEP", (String("WAKE AFTER = ") + ms).c_str());
        }
        //enter sleep
        sig_set(&SIG_WAKE_AFTER, ms);
        sig_set(&SIG_BEFORE_SLEEP, 1);
        
        sig_save(true);
        nap_enter_sleep();
    }
}

void nap_io_from_ulp()
{
    DEBUG("NAP-IO-TOUCH", String((ulp__touch & 0xFFFF)).c_str());
    DEBUG("NAP-IO-SW", String((ulp__switch & 0xFFFF)).c_str());
    DEBUG("NAP-IO-EN", String((ulp__encoder_state & 0xFFFF)).c_str());
    DEBUG("NAP-IO-EN_P", String((ulp__prev_encoder_state & 0xFFFF)).c_str());
    if ((ulp__touch & 0xFFFF) > 0)
    {
        io_touch_update();
    }
    if ((ulp__switch & 0xFFFF) > 0)
    {
        io_sw_update(); //TODO: THIS MIGHT BE WRONG
    }
    if ((ulp__encoder_state & 0xFFFF) != (ulp__prev_encoder_state & 0xFFFF))
    {
        io_encoder_update(ulp__encoder_state, ulp__prev_encoder_state);
    }
}

#define WAKE_TIMER 1
#define WAKE_ULP 2
#define WAKE_NONE 4
void nap_wake()
{
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_TIMER:
        sig_set(&SIG_WAKE, WAKE_TIMER);
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        sig_set(&SIG_WAKE, WAKE_ULP);
        break;
    default:
        sig_set(&SIG_WAKE, WAKE_NONE);
        break;
    }
}

#endif