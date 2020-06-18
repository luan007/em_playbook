#include "hardware.h";
#include "network.h";
#include "drawbin.h";
#include "luasys.h";

#include "app-engine.h";

void setup()
{
  setup_hardware();
  setup_serial();
  setup_fs();
  // setup_lua();
}

void loop()
{
  Serial.print(digitalRead(DT));
  Serial.print(":");
  Serial.print(digitalRead(CLK));
  Serial.print(":");
  Serial.println(digitalRead(CAP_ALERT));
  delay(50);
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