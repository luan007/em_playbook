#include "esp32/ulp.h"
// include ulp header you will create
#include "ulp_main.h"
// must include ulptool helper functions also
#include "ulptool.h"
#include "driver/rtc_io.h"
#include "driver/gpio.h"

#include <Wire.h>
#include <SparkFun_CAP1203_Registers.h>
#include <SparkFun_CAP1203_Types.h>
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <ESP32Encoder.h>

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

CAP1203 sensor;
ESP32Encoder encoder;
long last_user_interaction = 0;


// Unlike the esp-idf always use these binary blob names
extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");

static void init_run_ulp(uint32_t usec);

void printBin(uint16_t inByte)
{
    for (int b = 16; b >= 0; b--)
    {
        Serial.print(bitRead(inByte, b));
    }
}

int CTRL_ENCODER = 0;
int CTRL_TOUCH_BTN = 0;
int CTRL_TOUCH = 0;
int CTRL_CORE_CLICK = 0;
long CTRL_CORE_PRESS = 0;
int CTRL_CORE_HOLD = 0;
int CTRL_CORE_HOLD__triggered = 0;

void user_interaction()
{
    last_user_interaction = millis() + 1; //sanity
}

void encoder_update(int A, int B)
{
    CTRL_ENCODER = 0;
    if (A >= 0 && B >= 0)
    {
        A = A & 0xffff;
        B = B & 0xffff;
        if ((A == 1 && B == 3) || (A == 2 && B == 0))
        {
            CTRL_ENCODER = 1;
        }
        else if ((A == 0 && B == 1) || (A == 3 && B == 2))
        {
            CTRL_ENCODER = 1;
        }
        else if ((A == 1 && B == 0) || (A == 2 && B == 3))
        {
            CTRL_ENCODER = -1;
        }
        else if ((A == 3 && B == 1) || (A == 0 && B == 2))
        {
            CTRL_ENCODER = -1;
        }
    }
    else
    {
        int dir = encoder.getCount();
        CTRL_ENCODER = dir;
        CTRL_ENCODER = CTRL_ENCODER > 0 ? 1 : (CTRL_ENCODER < 0 ? -1 : CTRL_ENCODER);
        encoder.clearCount();
    }
    if (CTRL_ENCODER != 0)
    {
        user_interaction();
    }
}

void touch_update()
{
    int _prev_touch = CTRL_TOUCH;
    CTRL_TOUCH_BTN = 0;
    CTRL_TOUCH = 0;

    if (sensor.isLeftTouched() == true)
    {
        CTRL_TOUCH = 1;
    }
    if (sensor.isMiddleTouched() == true)
    {
        CTRL_TOUCH = 2;
    }
    if (sensor.isRightTouched() == true)
    {
        CTRL_TOUCH = 3;
    }
    if (_prev_touch != CTRL_TOUCH)
    {
        CTRL_TOUCH_BTN = _prev_touch;
    }

    if (CTRL_TOUCH != 0 || CTRL_TOUCH_BTN > 0)
    {
        user_interaction();
    }
}

void press_button_update()
{
    CTRL_CORE_CLICK = 0;
    CTRL_CORE_HOLD = 0;
    if (CTRL_CORE_PRESS == 0 && digitalRead(SW) == 0)
    {
        CTRL_CORE_PRESS = millis();
        CTRL_CORE_HOLD__triggered = 0;
    }
    if (digitalRead(SW) == 1)
    {
        if (CTRL_CORE_PRESS > 0 && (millis() - CTRL_CORE_PRESS) > 10)
        {
            CTRL_CORE_CLICK = 1;
        }
        CTRL_CORE_PRESS = 0;
    }

    if (CTRL_CORE_HOLD__triggered == 0 && CTRL_CORE_PRESS > 0 && (millis() - CTRL_CORE_PRESS) > 2000)
    {
        CTRL_CORE_HOLD = 1;
        CTRL_CORE_HOLD__triggered = 1;
    }

    if (CTRL_CORE_PRESS > 0)
    {
        user_interaction();
    }

}

void _wake_from_ulp()
{
    last_user_interaction = 1;
    if ((ulp__touch & 0xFFFF) > 0)
    {
        Serial.println("Touch");
        touch_update();
    }
    else if ((ulp__switch & 0xFFFF) > 0)
    {
        Serial.println("Switch");
        CTRL_CORE_PRESS = -10;
    }
    else if ((ulp__encoder_state & 0xFFFF) != (ulp__prev_encoder_state & 0xFFFF))
    {
        Serial.println("Encoder");
        encoder_update(ulp__encoder_state, ulp__prev_encoder_state);
    }
}

void _wake_from_timer()
{
    Serial.println("Routine Update");
    last_user_interaction = millis() + 8000; //you have 2 sec to op
}

void _sleep()
{
    if (last_user_interaction == 0 || (last_user_interaction > 0 && millis() - last_user_interaction > 3000))
    {
        esp_sleep_enable_timer_wakeup(1000 * 1000 * 15); // compute scheduled update
        init_run_ulp(500);
    }
    yield();
}

void setup()
{
    // REG_CLR_BIT(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN);

    Serial.begin(115200);
    Wire.begin();
    if (sensor.begin() == false)
    {
        Serial.println("Not connected. Please check connections and read the hookup guide.");
    }
    sensor.setSensitivity(SENSITIVITY_128X);
    sensor.setInterruptEnabled();

    Serial.println("Wake..");

    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_TIMER:
        Serial.println("Wakeup caused by timer");
        _wake_from_timer();
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        Serial.println("Wakeup caused by ULP program");
        _wake_from_ulp();
        break;
    default:
        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
        break;
    }
    pinMode(SW, INPUT);
    pinMode(DT, INPUT);
    pinMode(CLK, INPUT);
    pinMode(CAP_ALERT, INPUT);
    encoder.attachHalfQuad(CLK, DT);
}

void loop()
{
    // _sleep();
    if (CTRL_ENCODER != 0)
    {
        Serial.println("ENCODER");
        Serial.println(CTRL_ENCODER);
    }
    if (CTRL_TOUCH_BTN != 0)
    {
        Serial.println("CTRL_TOUCH_BTN");
        Serial.println(CTRL_TOUCH_BTN);
    }
    if (CTRL_CORE_CLICK != 0)
    {
        Serial.println("CLICK");
        Serial.println(CTRL_CORE_CLICK);
    }
    if (CTRL_CORE_HOLD != 0)
    {
        Serial.println("LONG HOLD");
        Serial.println(CTRL_CORE_HOLD);
    }
    encoder_update(-1, -1);
    touch_update();
    press_button_update();
    yield();
    delay(1);
    _sleep();
}

static void init_run_ulp(uint32_t usec)
{

    ulp_set_wakeup_period(0, usec);

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

    Serial.println("Prep Sleep");
    esp_deep_sleep_start();
}
