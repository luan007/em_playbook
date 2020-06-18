
#include "io.h"
#include "network.h"
#include "drawbin.h"
#include "luasys.h"
#include "app-engine.h"
#include "nap.h"

void setup()
{
  setup_io();
  nap_wake();
}

void loop()
{
  io_loop();
  nap_loop();
  // ensure_network();
  // if (ensure_network() == 1) //connected to net
  // {
  //   app_lifecycle();
  //   // download_inflate("demo", "http://192.168.1.183:9898/app/demo");
  //   // bin_smart_draw("/demo/root.bin", 800, 600, 0, 0, 800, 600, 0, 0);
  // }

  // if (Serial.available() && Serial.read() == 'F')
  // {
  //   USE_FS.end();
  //   USE_FS.format();
  //   ESP.restart();
  // }

  // esp_sleep_enable_timer_wakeup(10 * 1000000);
  // esp_sleep_enable_ext1_wakeup(1ULL << DT, ESP_EXT1_WAKEUP_ANY_HIGH);
  // Serial.println("ready to sleep");
  // esp_deep_sleep_start();
}