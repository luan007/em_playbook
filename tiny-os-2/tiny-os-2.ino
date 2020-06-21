/*
* v0.0.2
* Mike Luan
*/
#define CPU 1
#include "defs.h"
#include "nap.h"

void TaskMain(void *pvParameters);

void setup()
{
  Serial.begin(115200);

  xTaskCreatePinnedToCore(
      TaskMain, "TaskMain",
      1024, NULL, 2, NULL, CPU);
}

void loop()
{
  // Serial.println("Test from Loop");
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
