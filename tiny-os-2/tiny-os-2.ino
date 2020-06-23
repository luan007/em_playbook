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
#include "hal-display.h"
#include "app-engine.h"
#include "metal-renderer.h"

#define DEV_MODE 1

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
  display_sig_register();
  app_sig_register();
  base_subsys_init();

#ifdef DEV_MODE
  //DEV MODE
  // signal_raise(&SIG_FLUSH_CONFIG, 1);
  // signal_raise(&SIG_FLUSH_SIGS, 1);
#endif

  hal_io_setup();
  hal_fs_setup();
  nap_wake_sequence();

  // signal_raise(&SIG_WIFI_REQ, WIFI_REQ_AP_CONFIG);

  //for debug only

  SIG_USER_ACTION.debug_level = -1;
  SIG_NEXT_SLEEP.debug_level = -1;

  hal_network_loop();

  if (SIG_SYS_BROKE.value > 0 || (SIG_WAKE_REASON.value == WAKE_REASON_NONE))
  {
    signal_raise(&SIG_TIME_VALID, -1); //bad time due to power loss
  }

  //let us multi task
  xTaskCreatePinnedToCore(
      HAL_NET_IO_LOOP, "HAL_NET_IO_LOOP",
      10240, NULL, 2, NULL, CPU);

  DEBUG("APP GOOD DELTA", String(r_secs() - SIG_APP_GOOD.value).c_str());
  if (SIG_SYS_BROKE.value > 0 || (SIG_WAKE_REASON.value == WAKE_REASON_TIMER && (SIG_APP_GOOD.value <= 0 || (r_secs() - SIG_APP_GOOD.value < -1000000) || (r_secs() - SIG_APP_GOOD.value > CFG_APP_DUE.value64)))) //timer wake up check for app update
  {
    signal_raise(&SIG_APP_UPDATOR_REQUEST, 1);
  }
  // _graphics_commit += 1;
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

int sw_hold_debounce = 0;
void ux_loop()
{
  if (SIG_SW_CLICK.value > 0)
  {
    signal_resolve(&SIG_WIFI_RETRY, 0);
  }
  if (SIG_SW_HOLD.value > 0 && (millis() - sw_hold_debounce > 500))
  {
    sw_hold_debounce = millis();
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
  if (SIG_TOUCH_CLICK.value == 3)
  {
    signal_raise(&SIG_APP_UPDATOR_REQUEST, 1);
    // _graphics_commit += 1;
    // Serial.println("COMMIT DRAW");
  }
}

void loop()
{
  hal_io_loop();
  ux_loop();
  //backup render
  app_updator_loop();

  app_render_loop();
  display_render_loop();
  metal_render_handler();
  base_subsys_loop();
  nap_loop();
}
