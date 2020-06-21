/*
* v0.0.2
* Mike Luan
*/
#define CPU 1
#include "defs.h"
#include "nap.h"
#include "hal-io.h"
#include "hal-fs.h"
#include "hal-network.h"

void TaskMain(void *pvParameters);

// SIGNAL(TESTQ, "Test signal", SIGNAL_VIZ_APP | SIGNAL_VIZ_OS | SIGNAL_VIZ_USER, SIGNAL_PRESIST_POWERLOSS, 0)
// CONFIG(DUMMY, "Dummy Test", 394, "Hello world")

void setup()
{
  REG_CLR_BIT(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN); //stop ULP immediately
  Serial.begin(115200);

  DEBUG("BOOT", String(millis()).c_str()); //ms


  hal_io_sig_register();
  hal_fs_sig_register();
  hal_network_sig_register();
  base_subsys_init();


  hal_io_setup();
  hal_fs_setup();
  nap_wake_sequence();


  //let us multi task
  xTaskCreatePinnedToCore(
      HAL_IO_LOOP, "HAL_IO_LOOP",
      10240, NULL, 2, NULL, CPU);
}

void loop()
{
  base_subsys_loop();
  nap_loop();
  //ready to sleep
  // Serial.println("Test from Loop");
}

void HAL_IO_LOOP(void *pvParameters)
{
  (void)pvParameters;
  while (1)
  {
    hal_io_loop();
    vTaskDelay(15); //not too frequent
  }
}
