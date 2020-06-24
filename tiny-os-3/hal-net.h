#ifndef GUARD_NET
#define GUARD_NET
#define ESP32
#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include "hal-io.h"
#include "hal-nap.h"
#include "hal-fs.h"
#include "shared.h"
#include "time.h"
#include "src/untar.h"

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 28800;
const int daylightOffset_sec = 0;

#define WIFI_CONFIG 10
#define WIFI_CONNECTING 2
#define WIFI_IDLE 0
#define WIFI_FAILED -1
#define WIFI_SUCC 1
#define TIME_SYNCING 2
#define TIME_SUCC 1
#define TIME_FAILED -1

SIGNAL(WIFI, SIG_IMMEDIATE, SIG_RUNTIME, 0)
SIGNAL(TIME, SIG_IMMEDIATE, SIG_RUNTIME, 0)
SIGNAL(WIFI_TRY, SIG_ALL, SIG_POWERLOSS, 0)

WiFiManagerParameter server_addr_param("server", "application server", "", 40);
WiFiManager wm; // global wm instance

int net_wipe()
{
    DEBUG("WIFI", "WIPING SETTINGS");
    wm.resetSettings();
    wm.erase(true);
}

bool _net_config_time(int tout)
{
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm timeinfo;
    long timeout = millis() + tout;
    bool result = true;
    while (!getLocalTime(&timeinfo))
    {
        Serial.print(".");
        if (millis() > timeout)
        {
            result = false;
            break;
        }
    }
    if (result)
    {
        //// January 21, 2014 at 3am you would call:
        // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
        Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
        rtc.adjust(DateTime(timeinfo.tm_year + 1970, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
    }
    return result;
}

bool net_has_wifi_config()
{
    return wm.getWiFiIsSaved();
}

bool _first_connect = true;
int net_wifi_connect(bool sneaky_time_config = true)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        sig_set(&SIG_WIFI, WIFI_SUCC);
        return 1;
    }
    if (_first_connect)
    {
        WiFi.mode(WIFI_STA);
        _first_connect = false;
    }
    sig_set(&SIG_WIFI, WIFI_CONNECTING);
    wm.setConnectTimeout(5); //20sec for connection                      //burst connect
    wm.setEnableConfigPortal(false);
    // wm.setCleanConnect(true);
    if (!net_has_wifi_config())
    {
        //no config
        sig_set(&SIG_WIFI_TRY, ((SIG_WIFI_TRY.value + 100) > 1000) ? 1000 : (SIG_WIFI_TRY.value + 100));
        sig_set(&SIG_WIFI, WIFI_FAILED);
        return -1;
    }
    else if (wm.autoConnect())
    {
        if (sneaky_time_config)
        {
            _net_config_time(500); //500ms MAX
        }
        sig_set(&SIG_WIFI_TRY, 0);
        sig_set(&SIG_WIFI, WIFI_SUCC);
        return 1;
    }
    else
    {
        sig_set(&SIG_WIFI_TRY, SIG_WIFI_TRY.value + 1);
        sig_set(&SIG_WIFI, WIFI_FAILED);
        return -1;
    }
}

int net_update_time()
{
    if (net_wifi_connect(false) <= 0)
    {
        return -1; //failed to config time
    }
    sig_set(&SIG_TIME, TIME_SYNCING);
    bool result = _net_config_time(5000);
    sig_set(&SIG_TIME, result ? TIME_SUCC : TIME_FAILED);
    return result ? 1 : -1;
}

bool net_wifi_config()
{
    bool result = false;
    wm.setConfigPortalBlocking(false);
    wm.startConfigPortal("[ EMPaper_CFG ]");
    sig_set(&SIG_WIFI_TRY, 0);
    uint32_t start_time = millis();
    sig_set(&SIG_WIFI, WIFI_CONFIG);
    hal_io_loop();
    sig_clear(&SIG_SW_CLICK, 0);
    while (true)
    {
        if (wm.process())
        {
            result = true;
            break;
        }
        if (millis() - start_time > (240 * 1000) ||
            (SIG_SW_CLICK.triggered > 0 && SIG_SW_CLICK.value > 0))
        {
            result = false;
            break;
        }
        hal_io_loop();
    }
    if (!result)
    {
        wm.stopConfigPortal();
    }
    sig_set(&SIG_WIFI, result ? WIFI_SUCC : WIFI_FAILED);

    nap_set_sleep_duration(1000);
    nap_try_sleep(true);
    return result;
}

#endif