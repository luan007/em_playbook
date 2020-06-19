
#include "io.h"
#include "network.h"
#include "display.h"
#include "luasys.h"
#include "app-engine.h"
#include "nap.h"

void setup()
{
  power_eink(1);
  setup_serial();
  setup_fs();
  setup_io();
  nap_wake();
  dbg_print("WAKE");
}

void loop()
{
  if (SIG_ENCODER_DELTA != 0)
  {
    dbg_print(String(SIG_ENCODER_DELTA) + " - ENCODER - " + String(millis()));
  }
  if (SIG_TOUCH_CLICK != 0)
  {
    dbg_print(String(SIG_TOUCH_CLICK) + " - TOUCH - " + String(millis()));
  }
  if (SIG_GOING_SLEEP)
  {
    dbg_print("> SLEEP");
  }
  io_loop();
  nap_loop();

  // _cur_rot = (digitalRead(CLK) << 0) | (digitalRead(DT) << 1);

  // ensure_network();
  // if (ensure_network() == 1) //connected to net
  // {
  //   app_lifecycle();
  //   // download_inflate("demo", "http://192.168.1.183:9898/app/demo");
  //   // bin_smart_draw("/demo/root.bin", 800, 600, 0, 0, 800, 600, 0, 0);
  // }

  if (Serial.available())
  {
    switch (Serial.read())
    {
    case 'F':
      USE_FS.end();
      USE_FS.format();
      ESP.restart();
      break;
    case 'U':
      if (ensure_network() == 1)
      { 
        app_version_updator();
      }
      break;
    case 'R':
      app_full_refresh();
      break;
    }
  }

  // esp_sleep_enable_timer_wakeup(10 * 1000000);
  // esp_sleep_enable_ext1_wakeup(1ULL << DT, ESP_EXT1_WAKEUP_ANY_HIGH);
  // Serial.println("ready to sleep");
  // esp_deep_sleep_start();
}