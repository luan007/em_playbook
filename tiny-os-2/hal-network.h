#ifndef GUARD_NET
#define GUARD_NET
#define ESP32
#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include "defs.h"
#include "hal-io.h"
#include "hal-fs.h"
#include "untar.h"

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

#define WIFI_STATE_NO_CONFIG -2
#define WIFI_STATE_IDLE 0
#define WIFI_STATE_FAILED -3
#define WIFI_STATE_NO_MORE_TRY -4
#define WIFI_STATE_CONNECTED 1
#define WIFI_STATE_CONNECTING -1
#define WIFI_REQ_AP_CONFIG -999

SIGNAL(WIFI_REQ, "request wifi access", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_RUNTIME, 0)
SIGNAL(WIFI_WIPE, "wipe settings", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_POWERLOSS, 0)
SIGNAL(PORTAL_STATE, "portal status", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_RUNTIME, 0) //0 = shutdown, 1 = alive
SIGNAL(WIFI_STATE, "wifi status", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_RUNTIME, 0)
SIGNAL(WIFI_RETRY, "wifi reconnect attempts", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_POWERLOSS, 0)
SIGNAL(LAST_UPDATE, "last update time in real seconds", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_POWERLOSS, 0)
CONFIG(UPDATE_INTERVAL, "update interval", 30, "")
CONFIG(SERVER_ROOT, "server root", 0, "http://192.168.9.104:9898/")
CONFIG(MAX_CON_DUE, "singular con try", 5000, "") //give 5 seconds and die
CONFIG(MAX_CON_TRY, "max net retries", 5, "")

#define ESP_getChipId() ((uint32_t)ESP.getEfuseMac())

WiFiManager wm; // global wm instance
// //gets called when WiFiManager enters configuration mode
void configModeCallback(WiFiManager *myWiFiManager)
{
    auto SSID = myWiFiManager->getConfigPortalSSID();
}

int net_download_from_server(String fileName, String url)
{
    HTTPClient http;
    // dbg_print("HTTP REQ\n" + fileName + "\n" + url);
    // Serial.println("[HTTP] begin...\n");
    // Serial.println(fileName);
    // Serial.println(url);
    http.begin(url);
    // Serial.printf("[HTTP] GET...\n", url.c_str());
    // start connection and send HTTP header
    int httpCode = http.GET();
    int totalSize = 0;

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
                vPortYield();
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

//these needs to be migrated out
Tar<FS> tar(&USE_FS);

void _dbg_ls_dir(const char *c, int levels)
{
    File root = USE_FS.open(c);
    if (!root)
    {
        Serial.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                _dbg_ls_dir(file.name(), levels - 1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

int download_inflate(String pack_name, String url)
{
    //possible streaming option https://github.com/emelianov/untarArduino/blob/master/examples/WebUpdate/WebUpdate.ino
    String temp_pack = String("/") + pack_name + "_tmp";
    net_download_from_server(temp_pack, url);
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
        return tar._state == TAR_DONE ? 1 : -2;
    }
    else
    {
        Serial.println("Error open .tar file");
        return -1;
    }
    _dbg_ls_dir("/", 10);
    return 1;
}

int ensure_time()
{
    configTzTime("GMT-8", ntpServer);
    signal_raise(&SIG_TIME_VALID, 1);
}

int ensure_network()
{
    wm.getWiFiIsSaved();

    if (WiFi.status() == WL_CONNECTED)
        return 1;

    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    wm.setcallback(configModeCallback);
    wm.setConnectTimeout(30);                  // how long to try to connect for before continuing
    wm.setConfigPortalTimeout(0);              // auto close configportal after n seconds
    bool res = wm.autoConnect("🔧EMPaper_CFG"); // password protected ap
    if (!res)
    {
        return -1;
    }
    ensure_time();
    return 1;
}

void hal_network_loop()
{
    if (SIG_WIFI_WIPE.value == 1)
    {
        wm.resetSettings();
        WiFi.disconnect();
        signal_raise(&SIG_WIFI_RETRY, 0);
        signal_raise(&SIG_WIFI_WIPE, 0); //this should be resolve
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        signal_raise(&SIG_WIFI_STATE, WIFI_STATE_CONNECTED);
        signal_raise(&SIG_WIFI_RETRY, 0);
        return; //seems nothing to do
    }

    if (!wm.getWiFiIsSaved())
    {
        //empty
        signal_raise(&SIG_WIFI_RETRY, 0);
        signal_raise(&SIG_WIFI_STATE, WIFI_STATE_NO_CONFIG);
        return;
    }

    if (SIG_WIFI_RETRY.value >= CFG_MAX_CON_TRY.value64)
    {
        //give up!
        signal_raise(&SIG_WIFI_STATE, WIFI_STATE_NO_MORE_TRY);
        return;
    }

    if (SIG_WIFI_REQ.value > 0) // measure time
    {
        if (millis() - SIG_WIFI_REQ.value < 10000)
        {
            signal_raise(&SIG_WIFI_STATE, WIFI_STATE_CONNECTING);
            //connection trial
            wm.setConnectTimeout(CFG_MAX_CON_DUE.value64); //burst connect
            wm.setEnableConfigPortal(false);
            wm.autoConnect();
        }
        else
        {
            signal_raise(&SIG_WIFI_STATE, WIFI_STATE_FAILED);
            signal_raise(&SIG_WIFI_RETRY, SIG_WIFI_RETRY.value + 1);
            //dead
            //we give up, this counts for failed attempt - only one full reboot accounts
            return;
        }
        // wm.setConfigPortalBlocking(false);
    }
    else if (SIG_WIFI_REQ.value == 0)
    {
        //eject
        wm.stopConfigPortal();
        WiFi.disconnect();
        signal_raise(&SIG_PORTAL_STATE, 0);
        //network abundoned
        return;
    }
    else if (SIG_WIFI_REQ.value == WIFI_REQ_AP_CONFIG)
    {
        //start auto configurator
        if (SIG_PORTAL_STATE.value == 0)
        {
            wm.setConfigPortalBlocking(false);
            wm.startConfigPortal("🔧EMPaper_CFG");
            signal_raise(&SIG_PORTAL_STATE, millis()); //sometime later, I'd kill myself
        }
        else {
            //maintain
            wm.process();
        }
    }
}

void hal_network_sig_register()
{
    signal_register(&SIG_PORTAL_STATE);
    signal_register(&SIG_WIFI_STATE);
    signal_register(&SIG_WIFI_REQ);
    signal_register(&SIG_WIFI_WIPE);
    signal_register(&SIG_WIFI_STATE);
    signal_register(&SIG_WIFI_RETRY);
    signal_register(&SIG_LAST_UPDATE);
    config_register(&CFG_SERVER_ROOT);
    config_register(&CFG_UPDATE_INTERVAL);
    config_register(&CFG_MAX_CON_TRY);
    config_register(&CFG_MAX_CON_DUE);
}

#endif