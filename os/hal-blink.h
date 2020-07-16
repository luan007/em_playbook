#include <Ticker.h>
#include "hal-io.h"
Ticker blinkLed;
int tickerCount;
void togglerLed(int *state)
{
    *state = *state + 1;
    if (*state > 150)
    {
        digitalWrite(LED1, HIGH);
        digitalWrite(LED2, HIGH);
        *state = 0;
    }
    else
    {
        digitalWrite(LED1, LOW);
        digitalWrite(LED2, LOW);
    }
}

void hal_blink_setup()
{
    pinMode(LED1, OUTPUT);
    digitalWrite(LED1, HIGH);
    pinMode(LED2, OUTPUT);
    digitalWrite(LED2, HIGH);
    blinkLed.attach_ms(10, togglerLed, &tickerCount);
}
