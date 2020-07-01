/*
    Put your ULP globals here you want visibility
    for your sketch. Add "ulp_" to the beginning
    of the variable name and must be size 'uint32_t'
*/
#include "Arduino.h"
//// points to the entry function in counter.c.
extern uint32_t ulp_entry;
//// pointer to counter in counter.c
extern uint32_t ulp__switch;
extern uint32_t ulp__touch;
extern uint32_t ulp__prev_encoder_state;
extern uint32_t ulp__encoder_state;
