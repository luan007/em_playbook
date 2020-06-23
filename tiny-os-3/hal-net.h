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

SIGNAL(WIFI, SIG_IMMEDIATE, SIG_RUNTIME, 0)

void net_update_time() {
    //warning, block
}

#endif