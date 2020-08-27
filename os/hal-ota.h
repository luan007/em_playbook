#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <WiFiAP.h>
#include <Update.h>
#include "shared.h"
#include "hal-display.h"
#include "app-engine.h"
#define OS_VERSION 10000
#define BUFSIZE 512
uint8_t FileBuf[BUFSIZE];
const char *host = "esp32";
const char *ssid = "EM-Paper-OTA";
//const char* password = "emergeLAB";
WebServer server(80);
const char *serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
String OTADebug;
boolean ledTrigger = false;
long ota_time = millis();
bool ota_config()
{
  DEBUG("START OTA", "");
  OTADebug = "   Connect To [ EM-Paper-OTA ] To Update Your Device.\n\n";
  OTADebug += "   Open Brower And Enter [ http://192.168.4.1 ] To Upload Your File.\n\n";
  OTADebug += "   \n\n";
  OTADebug += "   This Configurator Will Stay Online 240 Seconds\n\n";
  OTADebug += "   If You Want To Cancel, Press The Button Again\n\n";
  display_dbg_print(OTADebug);
  WiFi.softAP(ssid);
  sig_clear(&SIG_OTA, 0);
  sig_clear(&SIG_OTA_REQ, 0);
  IPAddress myIP = WiFi.softAPIP();
  if (!MDNS.begin(host))
  { //http://esp32.local
    DEBUG("Error setting up MDNS responder!", "");
  }
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  server.on(
      "/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart(); }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      OTADebug  = "  Start Upload The File \n\n";
      OTADebug += "  DO NOT TURN OFF THE DEVICE !!!\n\n";
      OTADebug += "  " + String(upload.filename.c_str()) + " ";
      display_dbg_print(OTADebug);
      if (!Update.begin()) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.remaining() % (128 * 1024) == 0) {
        OTADebug += ".";
        display_dbg_print(OTADebug);
        ota_time = millis();
      }
      if (ledTrigger) {
        LED_A_ON;
      } else {
        LED_A_OFF;
      }
      ledTrigger = !ledTrigger;
      Serial.print("-------------------");
      Serial.println(upload.currentSize);
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }

    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        OTADebug += "\n\n  Update Successed !!!";
        OTADebug += "\n\n  System Restart !!!";
        display_dbg_print(OTADebug);
      } else {
        OTADebug += "\n\n  Update Failed !!!";
        OTADebug += "\n\n  System Restart !!!";
        display_dbg_print(OTADebug);
        Update.printError(Serial);
        ESP.restart();
      }
      Serial.setDebugOutput(false);
    } else {
      //Serial.printf("Update Failed Unexpectedly (likely broken connection): status=%d\n", upload.status);
      OTADebug += "\n\n  Update Failed !!!";
      OTADebug += "\n\n   System Restart !!!";
      display_dbg_print(OTADebug);
      ESP.restart();
    } });
  server.begin();
  hal_io_loop();
  sig_clear(&SIG_SW_CLICK, 0);
  while (1)
  {
    server.handleClient();
    delay(1);
    yield();
    if (millis() - ota_time > (240 * 1000) || (SIG_SW_CLICK.triggered > 0 && SIG_SW_CLICK.value > 0))
    {
      OTADebug = "\n\n  Time Out Or Press The Button To Exit !!!";
      display_dbg_print(OTADebug);
      break;
    }
    hal_io_loop();
  }
  ESP.restart();
}

// int ota_download_pack(String url)
// {
//   String nameLoc = String("/") + name;
//   USE_FS.remove(nameLoc);
//   if (!net_download_from_server(nameLoc, url, 3))
//   {
//     DEBUG("Network Error", "");
//     return -1;
//   }
//   _dbg_ls_dir("/", 10);
//   DEBUG("Download OTA file sucessful", "");
//   return 1;
// }

#define OTA_PAPP "os-ota"
#define OTA_FOLDER "/os-ota"
#define OTA_LOCATION "/os-ota/ota.firmware"

int ota_upgrade_from_file(String otaFile)
{
  File f = USE_FS.open(otaFile, "r");
  if (!f)
  {
    Serial.println("Failed to open file for reading");
    f.close();
    _rm_recur(OTA_FOLDER); //BAD OTA FIRMWARE
    return -1;
  }
  Serial.print("Read from file: ");
  if (!f.available())
  {
    Serial.println("Failed to read file");
    f.close();
    _rm_recur(OTA_FOLDER); //BAD OTA FIRMWARE
    return -1;
  }

  if (!Update.begin(UPDATE_SIZE_UNKNOWN))
  {
    Serial.println("OTA begin error!");
    Update.printError(Serial);
    f.close();
    _rm_recur(OTA_FOLDER); //BAD OTA FIRMWARE
    return -1;
  }
  else
  {
    while (f.available())
    {
      f.read(FileBuf, BUFSIZE);
      f.peek();
      if (Update.write(FileBuf, BUFSIZE) != BUFSIZE)
      {
        Serial.println("OTA process error!");
        Update.printError(Serial);
        f.close();
        _rm_recur(OTA_FOLDER); //BAD OTA FIRMWARE
        return -1;
      }
    }
    if (Update.end(true))
    {
      Serial.println("Update Success: \nRebooting...\n");
    }
    else
    {
      Update.printError(Serial);
      f.close();
      return -1;
    }
  }
  f.close();
  ESP.restart();
  return 1;
}

int ota_default_update()
{
  if (app_mgr_package_healthy(OTA_FOLDER) != 1)
  {
    _rm_recur(OTA_FOLDER); //BAD OTA FIRMWARE
    return -2;             //failed
  }
  int ver = app_mgr_get_app_version(OTA_PAPP);
  if (ver <= 0)
  {
    _rm_recur(OTA_FOLDER); //BAD OTA FIRMWARE
    return -2;             //failed
  }
  if (ver == OS_VERSION)
  {
    return 0; //latest
  }
  if (ver < OS_VERSION)
  {
    _rm_recur(OTA_FOLDER); //BAD OTA FIRMWARE
    return -1;             //bad, this os is newer than firmware
  }
  sig_set(&SIG_AUTO_OTA, 1);
  int result = ota_upgrade_from_file(OTA_LOCATION);
  sig_set(&SIG_AUTO_OTA, result);
}

int ota_version()
{
  return OS_VERSION;
}