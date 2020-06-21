#ifndef _GUARD_H_NAP
#define _GUARD_H_NAP

#include "esp32/ulp.h"
#include "ulp/ulp_main.h"
#include "ulptool.h"
#include "driver/rtc_io.h"
#include "driver/gpio.h"
extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");

uint32_t nap_next_wake = 60 * 1000; //1min at a time

//kickstart ULP
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

// void _wake_from_ulp()
// {
//     if ((ulp__touch & 0xFFFF) > 0)
//     {
//         Serial.println("from touch");
//         io_touch_update();
//     }
//     else if ((ulp__switch & 0xFFFF) > 0)
//     {
//         Serial.println("from sw");
//         SIG_ENCODER_PRESS = -10;
//     }
//     else if ((ulp__encoder_state & 0xFFFF) != (ulp__prev_encoder_state & 0xFFFF))
//     {
//         Serial.println("from encoder");
//         io_encoder_update(ulp__encoder_state, ulp__prev_encoder_state);
//     }
//     SIG_WAKE_BY_INPUT = 1;
//     INTERACTION_TIMESTAMP = 1;
// }

// void _wake_from_timer()
// {
//     SIG_WAKE_BY_TIMER = 1;
//     Serial.println("Woke from Timer");
//     INTERACTION_TIMESTAMP = NAP_AFTER_INTERACTION / 2; //you have 2 sec to op - ready to sleep actually
// }

void nap_wake()
{
    REG_CLR_BIT(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN);
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_TIMER:
        Serial.println("Wakeup caused by timer");
        // _wake_from_timer();
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        Serial.println("Wakeup caused by ULP program");
        // _wake_from_ulp();
        break;
    default:
        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
        break;
    }
}

#endif