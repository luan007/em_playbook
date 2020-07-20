#ifndef _GUARD_H_BLINK
#define _GUARD_H_BLINK

#include <Ticker.h>

#define LED1 32
#define LED2 33

Ticker blinkLed;

typedef struct led_fx
{
  int channel;
  float constant;

  int blink_interval;
  float blink_strength;
  int blink_duration;
  int blink_invert;
  int blink_counter;

  float wave_strength;
  float wave_dir;
  float wave_pos;
  float wave_spd;
  float wave_pow;

  float easing;
  float current;
  float target;
};

struct led_fx LED_1 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
struct led_fx LED_2 = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

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
    if ((led->blink_invert == 0 && led->blink_counter < led->blink_duration) ||
        (led->blink_invert == 1 && led->blink_counter > led->blink_duration))
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
  if (id == 0)
  {
    return &LED_1;
  }
  if (id == 1)
  {
    return &LED_2;
  }
}

void hal_led_constant(int led, float constant) {
  get_led_state(led)->constant = constant;
}

void hal_led_blink(int led, float strength, int interval, int duration, int invert = 0) {
  auto led_fx = get_led_state(led);
  led_fx->blink_interval = interval;
  led_fx->blink_strength = strength;
  led_fx->blink_duration = duration;
  led_fx->blink_invert = invert;
}

void hal_led_wave(int led, float strength, float spd, float pow = 0) {
  auto led_fx = get_led_state(led);
  led_fx->wave_strength = strength;
  led_fx->wave_spd = spd;
  if (led_fx->wave_dir == 0) {
    led_fx->wave_dir = spd;
  }
  led_fx->wave_pow = pow;
}

void hal_led_easing(int led, float ease) {
  get_led_state(led)->easing = ease;
}

void toggleLed(/*int *state*/)
{
  compute_led(&LED_1);
  compute_led(&LED_2);
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
  
  hal_led_constant(1, 0);
  hal_led_constant(0, 0);

  hal_led_blink(0, 1, 100, 5);
  hal_led_easing(0, 0.3);
  hal_led_wave(0, 0.2, 0.005);

  blinkLed.attach_ms(10, toggleLed);
}


#endif