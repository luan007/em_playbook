#include "entry.h"

void setup()
{
    Serial.begin(115200);
    Serial.println("--------------------------------");
    Serial.println("--------------BOOT--------------");
    Serial.println("--------------------------------");
    sys_init();
}

void loop()
{
    if(SIG_ALLOW_LOOP.value == 0) {
        //kill right now
        //ERROR?
        sig_set(&SIG_NO_MORE_OP, 1);
        nap_try_sleep(true);
    }
    // hal_io_loop();
    // sys_tick();
}