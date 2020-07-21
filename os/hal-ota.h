#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <WiFiAP.h>
#include "shared.h"
#include "hal-display.h"

const char* host = "esp32";
const char* ssid = "EM-Paper-OTA";
//const char* password = "emergeLAB";
WebServer server(80);
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
String OTADebug;
boolean ledTrigger = false;
long ota_time = millis();
bool ota_config() {
  DEBUG("START OTA", "");
  OTADebug = "   Connect To [ EM-Paper ] To Update Your Device.\n\n";
  OTADebug += "   Open Brower And Enter [ http://192.168.4.1 ] To Upload Your File.\n\n";
  OTADebug += "   \n\n";
  OTADebug += "   This Configurator Will Stay Online 240 Seconds\n\n";
  OTADebug += "   If You Want To Cancel, Press The Button Again\n\n";
  display_dbg_print(OTADebug);
  WiFi.softAP(ssid);
  sig_clear(&SIG_OTA, 0);
  sig_clear(&SIG_OTA_REQ, 0);
  IPAddress myIP = WiFi.softAPIP();
  if (!MDNS.begin(host)) { //http://esp32.local
    DEBUG("Error setting up MDNS responder!", "");
  }
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
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
    }
  });
  server.begin();
  hal_io_loop();
  sig_clear(&SIG_SW_CLICK, 0);
  while (1) {
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
