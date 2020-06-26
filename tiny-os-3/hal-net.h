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
        rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
        sig_clear(&SIG_RTC_INVALID, 0);
    }
    return result;
}

bool net_has_wifi_config()
{
    return wm.getWiFiIsSaved();
}

bool _first_connect = true;
int net_wifi_connect(int internal_retries, bool sneaky_time_config = true)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return 1;
    }
    if (internal_retries == 0)
    {
        return -1;
    }
    if (_first_connect)
    {
        WiFi.mode(WIFI_STA);
        _first_connect = false;
    }
    wm.setCleanConnect(true);
    wm.setConnectTimeout(3); //20sec for connection                      //burst connect
    wm.setEnableConfigPortal(false);
    // wm.setCleanConnect(true);
    if (!net_has_wifi_config())
    {
        //no config
        return -1;
    }
    else if (wm.autoConnect())
    {
        if (sneaky_time_config)
        {
            _net_config_time(500); //500ms MAX
        }
        return 1;
    }
    else
    {
        delay(1000);
        return net_wifi_connect(internal_retries - 1, sneaky_time_config);
    }
}

int net_reset()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        WiFi.disconnect();
    }
}

int net_wifi_connect(bool sneaky_time_config = true)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        sig_set(&SIG_WIFI, WIFI_SUCC);
        return 1;
    }
    else
    {
        sig_set(&SIG_WIFI, WIFI_CONNECTING);
        if (net_wifi_connect(3, sneaky_time_config) <= 0)
        {
            sig_set(&SIG_WIFI_TRY, SIG_WIFI_TRY.value + 1);
            sig_set(&SIG_WIFI, WIFI_FAILED);
            return -1;
        }
        else
        {
            sig_clear(&SIG_WIFI_TRY, 0);
            sig_set(&SIG_WIFI, WIFI_SUCC);
            return 1;
        }
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

///////////DOWNLOADER

//these needs to be migrated out
Tar<FS> tar(&USE_FS);

int net_download_from_server(String fileName, String url)
{
    HTTPClient http;
    DEBUG("HTTP", (String("REQ\n") + fileName + "\n" + url).c_str());
    // Serial.println("[HTTP] begin...\n");
    // Serial.println(fileName);
    // Serial.println(url);
    http.setConnectTimeout(5000); //5sec
    http.setTimeout(5000);        //5sec
    http.begin(url);
    // Serial.printf("[HTTP] GET...\n", url.c_str());
    // start connection and send HTTP header
    int httpCode = http.GET();
    int totalSize = 0;
    DEBUG("HTTP", String(httpCode).c_str());
    if (httpCode > 0)
    {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        Serial.printf("[FILE] open file for writing - ");
        Serial.println(fileName);

        File file = USE_FS.open(fileName, FILE_WRITE);

        // file found at server
        if (httpCode == HTTP_CODE_OK)
        {
            // get lenght of document (is -1 when Server sends no Content-Length header)
            int len = http.getSize();

            // create buffer for read
            uint8_t buff[128] = {0};

            // get tcp stream
            WiFiClient *stream = http.getStreamPtr();

            // read all data from server
            while (http.connected() && (len > 0 || len == -1))
            {
                // get available data size
                size_t size = stream->available();
                if (size)
                {
                    // read up to 128 byte
                    int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                    // write it to Serial
                    //Serial.write(buff, c);
                    file.write(buff, c);
                    totalSize += c;
                    if (len > 0)
                    {
                        len -= c;
                    }
                }
                // yield();
            }
            // Serial.println();
            // Serial.println("[HTTP] connection closed or file end.\n");
            // Serial.println("[FILE] closing file\n");
            // Serial.println("[FILE] size:\n");
            // Serial.println(totalSize);
            file.close();
            http.end();
            // dbg_print("DOWNLOADED\n" + String(totalSize));
            return 1;
        }
        else
        {
            http.end();
            return -2;
        }
    }
    else
    {
        http.end();
        return -1;
    }
}

int net_download_from_server(String fileName, String url, int retries)
{
    if (retries == 0)
    {
        return -1;
    }
    int result = net_download_from_server(fileName, url);
    if (result > 0)
    {
        return result;
    }
    else
    {
        return net_download_from_server(fileName, url, retries - 1);
    }
}

int net_download_then_inflate(String pack_name, String url)
{
    //possible streaming option https://github.com/emelianov/untarArduino/blob/master/examples/WebUpdate/WebUpdate.ino
    String temp_pack = String("/") + pack_name + "_tmp";
    USE_FS.remove(temp_pack);
    if (!net_download_from_server(temp_pack, url, 3))
    {
        Serial.println("Network Error");
        return -1;
    }
    _dbg_ls_dir("/", 10);
    File f = USE_FS.open(temp_pack, "r"); // Open source file
    String dest = String("/") + pack_name;
    if (f)
    {
        USE_FS.mkdir(dest);
        tar.open((Stream *)&f);                 // Pass source file as Stream to Tar object
        tar.dest(strdup((dest + "/").c_str())); // Set destination prefix to append to all files in archive. Should start with "/" for SPIFFS
        tar.extract();                          // Process with extraction
        f.close();
        USE_FS.remove(temp_pack.c_str());
        Serial.println(tar._state);
        return (tar._state == TAR_DONE || tar._state == TAR_SOURCE_EOF) ? 1 : -2;
    }
    else
    {
        Serial.println("Error open .tar file");
        return -1;
    }
    _dbg_ls_dir("/", 10);
    return 1;
}

#endif