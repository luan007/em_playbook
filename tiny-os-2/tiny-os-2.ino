/*
* v0.0.2
* Mike Luan
*/
#define CPU 1
#include "defs.h"
#include "nap.h"
#include "hal-io.h"

void TaskMain(void *pvParameters);

// SIGNAL(TESTQ, "Test signal", SIGNAL_VIZ_APP | SIGNAL_VIZ_OS | SIGNAL_VIZ_USER, SIGNAL_PRESIST_POWERLOSS, 0)
// CONFIG(DUMMY, "Dummy Test", 394, "Hello world")

void setup()
{
  Serial.begin(115200);

  hal_io_setup();
  base_subsys_init();

  nap_init();


  // signal_raise(&SIG_FLUSH_SIGS, 0, NULL);
  // signal_raise(&SIG_TESTQ, 392, NULL);

  //start os
  xTaskCreatePinnedToCore(
      TaskMain, "TaskMain",
      1024, NULL, 2, NULL, CPU);

}

void loop()
{
  //ready to sleep
  // Serial.println("Test from Loop");
  hal_io_loop();
  base_subsys_loop();
  nap_loop();
}

void TaskMain(void *pvParameters)
{
  (void)pvParameters;
  while (1)
  {
    // Serial.println("Test from Seperate Task");
    vTaskDelay(100);
  }
}
