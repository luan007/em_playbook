#include "book-sys.h"

void setup()
{
    Serial.begin(115200);
    sys_init();
}

void loop()
{
    hal_io_loop();
    sys_tick();
}