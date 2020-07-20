#include <Ticker.h>
// #include "hal-io.h"
// #include "shared.h"

#define LED1 32
#define LED2 33

Ticker blinkLed;

typedef struct led_fx
{
    int channel = 0;

    float constant = 0;

    int blink_interval = 0;
    float blink_strength = 0;
    int blink_duration = 0;
    int blink_invert = 0;
    int blink_counter = 0;

    float wave_strength = 0;
    float wave_dir = 0;
    float wave_pos = 0;
    float wave_spd = 0;
    float wave_pow = 0;

    float easing = 0.0;
    float current = 0.0;
    float target = 0.0;
};

struct led_fx LED_1 = {0};
struct led_fx LED_2 = {1};

inline void compute_led(struct led_fx *led)
{
    led->target = led->constant;
    if (led->blink_interval > 0)
    {
        led->blink_counter++;
        if (led->blink_counter > led->blink_interval)
        {
            led->blink_counter = 0; //reset
        }
        if ((led->blink_invert == 0 && led->blink_counter > led->blink_duration) ||
            (led->blink_invert == 1 && led->blink_counter < led->blink_duration))
        {
            led->target += led->blink_strength;
        }
    }
    if (led->wave_strength > 0)
    {
        led->wave_pos += led->wave_dir;
        if (led->wave_pos > 1)
        {
            led->wave_pos = 1;
            led->wave_dir = -led->wave_spd;
        }
        else if (led->wave_pos < 0)
        {
            led->wave_pos = 0;
            led->wave_dir = led->wave_spd;
        }
        if (led->wave_pow > 0)
        {
            led->target += led->wave_strength * pow(led->wave_pos, led->wave_pow);
        }
        else
        {
            led->target += led->wave_strength * led->wave_pos;
        }
    }
    led->target = led->target > 1 ? 1 : led->target;
    if (led->easing > 0 && led->easing < 1)
    {
        led->current = led->current + (led->target - led->current) * led->easing;
    }
    else
    {
        led->current = led->target;
    }
    int led_pwr = led->current * 65535;
    if (led_pwr < 0 || led_pwr > 65535)
    {
        led_pwr = led_pwr < 0 ? 0 : (led_pwr > 65535 ? 65535 : led_pwr);
    }
    ledcWrite(led->channel, led_pwr);
}

inline led_fx *get_led_state(int id)
{
    //this is stupid
    //but should be rather fast as this runs in ms intervals
    if (id == 1)
    {
        return &LED_1;
    }
    if (id == 2)
    {
        return &LED_2;
    }
}

void hal_led_constant(int led, float constant) {
    get_led_state(led)->constant = constant;
}

void hal_led_blink(int led, float strength, int interval, int duration, int invert = 0) {
    auto led = get_led_state(led);
    led->blink_interval = interval;
    led->blink_strength = strength;
    led->blink_duration = duration;
    led->blink_invert = invert;
}

void hal_led_wave(int led, float strength, float spd, float pow = 0) {
    auto led = get_led_state(led);
    led->wave_strength = strength;
    led->wave_spd = spd;
    if(led->wave_dir == 0) {
        led->wave_dir = spd;
    }
    led->wave_pow = pow;
}

void hal_led_easing(int led, float ease) {
    get_led_state(led)->easing = ease;
}

void toggleLed(/*int *state*/)
{
    compute_led(&LED_1);
    compute_led(&LED_2);
    // if (*state > 150)
    // {
    //     digitalWrite(LED1, HIGH);
    //     digitalWrite(LED2, HIGH);
    //     *state = 0;
    // }
    // else
    // {
    //     digitalWrite(LED1, LOW);
    //     digitalWrite(LED2, LOW);
    // }
}

void hal_blink_setup()
{
    pinMode(LED1, OUTPUT);
    digitalWrite(LED1, HIGH);
    pinMode(LED2, OUTPUT);
    digitalWrite(LED2, HIGH);

    ledcSetup(0, 5000, 16);
    ledcSetup(1, 5000, 16);
    ledcAttachPin(LED1, 0);
    ledcAttachPin(LED2, 1);

    blinkLed.attach_ms(5, toggleLed);
}