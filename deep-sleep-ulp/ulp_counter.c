#ifdef _ULPCC_
#include <ulp_c.h>
#define debounce_limit 3
unsigned _switch = 0;
unsigned _touch = 0;

#define debounce(bit_in, true_state, debouncer) \
    if ((bit_in & 0x1) == (true_state & 0x1))   \
    {                                           \
        debouncer += 1;                         \
        if (debouncer > debounce_limit)         \
        {                                       \
            debouncer = debounce_limit;         \
        }                                       \
    }                                           \
    else                                        \
    {                                           \
        if (debouncer > 0)                      \
        {                                       \
            debouncer--;                        \
        }                                       \
    }

void entry()
{
    debounce(READ_RTC_REG(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S + 17, 1), 0x00, _switch);
    debounce(READ_RTC_REG(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S + 7, 1), 0x00, _touch);
    if (_switch == debounce_limit || _touch == debounce_limit)
    {
        wake();
        WRITE_RTC_FIELD(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN, 0);
        halt();
    }
}

#endif
