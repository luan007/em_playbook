#ifndef GUARD_HW
#define GUARD_HW
#include "FS.h"
#include "SPIFFS.h"
#include "FFat.h"
#define USE_FS FFat

void setup_fs()
{
  // USE_FS.format();
  USE_FS.begin();
}

#include <ESP32Encoder.h>
#include <SparkFun_CAP1203_Registers.h>
#include <SparkFun_CAP1203_Types.h>
#include <Wire.h>

#define LED1 32
#define LED2 33

#define SW 27
#define DT 13
#define CLK 14

#define CAP_ALERT 26
#define CAP_SDA 21
#define CAP_SCL 22

#define PWR_CONTROL 17
#define TPS_CONTROL 15
#define IT8951_RESET 16

ESP32Encoder encoder;
CAP1203 sensor;

void IRAM_ATTR swInterrupt()
{
  if (!digitalRead(SW))
  {
    //Serial.println("SSSSSSWWWWWW");
  }
}

void IRAM_ATTR capInterrupt()
{
  if (!digitalRead(CAP_ALERT))
  {
    Serial.println("CAP_ALERT");
  }
}


void setup_serial()
{
  Serial.begin(115200);
}

void setup_hardware()
{
  Wire.begin();

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);

  pinMode(SW, INPUT);
  pinMode(DT, INPUT);
  pinMode(CLK, INPUT);

  pinMode(CAP_ALERT, INPUT);
  attachInterrupt(SW, swInterrupt, FALLING);
  attachInterrupt(CAP_ALERT, capInterrupt, FALLING);
  encoder.attachHalfQuad(CLK, DT);
  if (sensor.begin() == false)
  {
    Serial.println("Not connected. Please check connections and read the hookup guide.");
    while (1)
      ;
  }
  else
  {
    Serial.println("Connected!");
  }
  sensor.setSensitivity(SENSITIVITY_128X);
}


#endif
