/*
* v0.0.2
* Mike Luan
*/
#define CPU 1
#include "defs.h"
#include "nap.h"

void TaskMain(void *pvParameters);

SIGNAL(TEST, "Test signal", SIGNAL_VIZ_APP | SIGNAL_VIZ_OS | SIGNAL_VIZ_USER, SIGNAL_PRESIST_POWERLOSS, 0)

void setup()
{
  Serial.begin(115200);

  signal_register(&TEST);

  signal_presist_init();

  signal_raise(&TEST, 2, NULL);




  //start os
  xTaskCreatePinnedToCore(
      TaskMain, "TaskMain",
      1024, NULL, 2, NULL, CPU);

}

void loop()
{
  // Serial.println("Test from Loop");
  signal_presist_update();
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
