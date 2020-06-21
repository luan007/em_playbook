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
  signal_raise(&SIG_FLUSH_CONFIG, 1);
  signal_raise(&SIG_FLUSH_SIGS, 1);

  hal_io_setup();
  hal_fs_setup();
  nap_wake_sequence();

  // signal_raise(&SIG_WIFI_REQ, WIFI_REQ_AP_CONFIG);

  //for debug only

  SIG_USER_ACTION.debug_level = -1;
  SIG_NEXT_SLEEP.debug_level = -1;

  //let us multi task
  xTaskCreatePinnedToCore(
      HAL_NET_IO_LOOP, "HAL_NET_IO_LOOP",
      10240, NULL, 2, NULL, CPU);

  hal_network_loop(); //initialise
}

void factory_reset()
{
  DEBUG("FACTORY RESET", "WARNING - FACTORY RESET REQUESTED", 1);
  signal_raise(&SIG_FORMAT_FS, 1);
  signal_raise(&SIG_FLUSH_CONFIG, 1);
  signal_raise(&SIG_FLUSH_SIGS, 1);
  signal_raise(&SIG_WIFI_WIPE, 1);
  base_force_tick();
  ESP.restart();
}

void ux_loop()
{
  if (SIG_SW_HOLD.value > 0)
  {
    //hold to toggle network config mode
    signal_raise(&SIG_WIFI_REQ, (SIG_WIFI_REQ.value == WIFI_REQ_AP_CONFIG) ? 0 : WIFI_REQ_AP_CONFIG);
  }
  if (SIG_SW_SHOLD.value > 0)
  {
    signal_raise(&SIG_FAC_RESET, 1);
  }
  if (SIG_FAC_RESET.value > 0)
  {
    factory_reset();
  }
}

void loop()
{
  hal_io_loop();
  ux_loop();
  base_subsys_loop();
  nap_loop();
  taskYIELD();
  //ready to sleep
  // Serial.println("Test from Loop");
}

void HAL_NET_IO_LOOP(void *pvParameters)
{
  (void)pvParameters;
  while (1)
  {
    hal_network_loop();
    //ensures we get signal
    vTaskDelay(1); //not too frequent - low prio
  }
}
