#ifndef GUARD_NET
#define GUARD_NET

#include <WiFi.h>
#include <WiFiManager.h> //https://github.com/khoih-prog/ESP_WiFiManager
#include "io.h"
#include "display.h"
#include <HTTPClient.h>
#include <FS.h>
// #include <ESP32-targz.h>
#include "untar.h"

#define ESP_getChipId() ((uint32_t)ESP.getEfuseMac())

WiFiManager wm; // global wm instance
// //gets called when WiFiManager enters configuration mode
void configModeCallback(WiFiManager *myWiFiManager)
{
    dbg_print("AP MODE ENTERED\n\nSSID\n\n" + myWiFiManager->getConfigPortalSSID());
}

int download_from_server(String fileName, String url)
{
    HTTPClient http;

    dbg_print("HTTP REQ\n" + fileName + "\n" + url);

    Serial.println("[HTTP] begin...\n");
    Serial.println(fileName);
    Serial.println(url);
    http.begin(url);

    Serial.printf("[HTTP] GET...\n", url.c_str());
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
                yield();
            }

            Serial.println();
            Serial.println("[HTTP] connection closed or file end.\n");
            Serial.println("[FILE] closing file\n");
            Serial.println("[FILE] size:\n");
            Serial.println(totalSize);
            file.close();
            http.end();
            dbg_print("DOWNLOADED\n" + String(totalSize));
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
    download_from_server(temp_pack, url);
    dbg_print("DECOMPRESSING " + temp_pack + " -> " + "/" + pack_name);
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
        dbg_print("CLEAN_UP " + temp_pack);
        USE_FS.remove(temp_pack.c_str());
        Serial.println(dest);
    }
    else
    {
        Serial.println("Error open .tar file");
    }
    _dbg_ls_dir("/", 10);
}

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

int ensure_time()
{
    configTzTime("GMT-8", ntpServer);
}

int ensure_network()
{
    if (WiFi.status() == WL_CONNECTED)
        return 1;
    Serial.println("Booting Network stack");
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    wm.setClass("invert");
    wm.setAPCallback(configModeCallback);
    wm.setConnectTimeout(30);     // how long to try to connect for before continuing
    wm.setConfigPortalTimeout(0); // auto close configportal after n seconds
    dbg_print("Network Stack: Booting..");
    bool res = wm.autoConnect("🔧EMPaper_CFG"); // password protected ap
    if (!res)
    {
        dbg_print("Failed to config network, rebooting.");
        Serial.println("Gave up :(");
        ESP.restart();
        return -1;
    }
    dbg_print("Connected to WiFi");
    ensure_time();
    return 2;
}
#endif