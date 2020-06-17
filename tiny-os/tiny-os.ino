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
  if (ensure_network() == 1) //connected to net
  {
    app_lifecycle();
    // download_inflate("demo", "http://192.168.1.183:9898/app/demo");
    // bin_smart_draw("/demo/root.bin", 800, 600, 0, 0, 800, 600, 0, 0);
  }

  if (Serial.available() && Serial.read() == 'F')
  {
    USE_FS.end();
    USE_FS.format();
    ESP.restart();
  }
}