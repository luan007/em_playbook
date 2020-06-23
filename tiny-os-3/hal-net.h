#ifndef GUARD_NET
#define GUARD_NET
#define ESP32
#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include "hal-io.h"
#include "hal-fs.h"
#include "shared.h"
#include "time.h"
#include "src/untar.h"

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

#define WIFI_CONFIG 10
#define WIFI_CONNECTING 2
#define WIFI_FAILED -1
#define WIFI_SUCC 1

SIGNAL(WIFI, SIG_IMMEDIATE, SIG_RUNTIME, 0)

int net_update_time() {
    //warning, block
    return -1;
}

void net_wifi_config() {
    sig_set(&SIG_WIFI, WIFI_CONFIG);
}

#endif