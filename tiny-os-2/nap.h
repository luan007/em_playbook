#ifndef _GUARD_H_NAP
#define _GUARD_H_NAP

#include "esp32/ulp.h"
#include "ulp/ulp_main.h"
#include "ulptool.h"
#include "driver/rtc_io.h"
#include "driver/gpio.h"
#include "defs.h"
#include "hal-io.h"

#define WAKE_REASON_ULP 2
#define WAKE_REASON_NONE 1
#define WAKE_REASON_TIMER 3

SIGNAL(WAKE_REASON, "Wake reason", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_RUNTIME, 0)
SIGNAL(BEFORE_SLEEP, "This will fire before sleep", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_ONCE_AUTO_ZERO, 0)

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");

uint32_t nap_next_wake = 60 * 1000; //1min at a time

//kickstart ULP

void nap_loop()
{
    if (SIG_BEFORE_SLEEP.value > 0)
    {
        //ready to sleep
        nap_enter_sleep(nap_next_wake); //TODO: THIS IS STILL WRONG
        //YOU SHOULD NEVER GET HERE (HALT)
    }
}

void nap_enter_sleep(uint32_t WAKE_DUR_SECONDS)
{
    rtc_gpio_init(GPIO_NUM_26);
    rtc_gpio_set_direction(GPIO_NUM_26, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_init(GPIO_NUM_27);
    rtc_gpio_set_direction(GPIO_NUM_27, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_init(GPIO_NUM_13);
    rtc_gpio_set_direction(GPIO_NUM_13, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_init(GPIO_NUM_14);
    rtc_gpio_set_direction(GPIO_NUM_14, RTC_GPIO_MODE_INPUT_ONLY);

    esp_sleep_enable_ulp_wakeup();

    ulp__switch = 0;
    ulp__touch = 0;
    ulp__encoder_state = 0;
    ulp__prev_encoder_state = 255;
    esp_err_t err = ulptool_load_binary(0, ulp_main_bin_start, (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));
    err = ulp_run((&ulp_entry - RTC_SLOW_MEM) / sizeof(uint32_t));
    if (err)
        Serial.println("Error Starting ULP Coprocessor");

    esp_sleep_enable_timer_wakeup(WAKE_DUR_SECONDS * 1000 * 15);
    // nap_schedule_next_wake();
    ulp_set_wakeup_period(0, 100);

    Serial.println("Entering Sleep Now.");
    esp_deep_sleep_start();
}

void nap_read_input_from_ulp()
{
    if ((ulp__touch & 0xFFFF) > 0)
    {
        io_touch_update();
    }
    else if ((ulp__switch & 0xFFFF) > 0)
    {
        io_sw_update(); //TODO: THIS MIGHT BE WRONG
    }
    else if ((ulp__encoder_state & 0xFFFF) != (ulp__prev_encoder_state & 0xFFFF))
    {
        io_encoder_update(ulp__encoder_state, ulp__prev_encoder_state);
    }
}

void nap_init()
{
    signal_register(&SIG_WAKE_REASON);
    signal_register(&SIG_BEFORE_SLEEP);

    REG_CLR_BIT(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN);
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_TIMER:
        signal_raise(&SIG_WAKE_REASON, WAKE_REASON_TIMER, "Wake from Timer");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        signal_raise(&SIG_WAKE_REASON, WAKE_REASON_ULP, "Wake from ULP");
        nap_read_input_from_ulp();
        break;
    default:
        signal_raise(&SIG_WAKE_REASON, WAKE_REASON_NONE, "Wake from RESET or Other");
        break;
    }
}

#endif