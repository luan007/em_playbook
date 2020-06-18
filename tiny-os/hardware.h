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
#define SW_GPIO GPIO_NUM_27
#define DT 13
#define DT_GPIO GPIO_NUM_13
#define CLK 14
#define CLK_GPIO GPIO_NUM_14

#define CAP_ALERT 26
#define CAP_ALERT_GPIO GPIO_NUM_26
#define CAP_SDA 21
#define CAP_SCL 22

#define PWR_CONTROL 17
#define TPS_CONTROL 15
#define IT8951_RESET 16

ESP32Encoder encoder;
CAP1203 sensor;

void IRAM_ATTR swInterrupt()
{
  Serial.println("SW");
}
void IRAM_ATTR encoderInterrupt()
{
  Serial.println("ENCODER");
  Serial.println(digitalRead(DT));
  Serial.println(digitalRead(CLK));
}
void IRAM_ATTR capInterrupt()
{
  Serial.println("CAP");
  Serial.println(sensor.isInterruptEnabled());
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

  // attachInterrupt(SW, swInterrupt, FALLING);
  // attachInterrupt(CAP_ALERT, capInterrupt, FALLING);
  // attachInterrupt(DT, encoderInterrupt, FALLING);
  // attachInterrupt(CLK, encoderInterrupt, FALLING);
  encoder.attachHalfQuad(CLK, DT);

  if (sensor.begin() == false)
  {
    Serial.println("Not connected. Please check connections and read the hookup guide.");
  }
  sensor.setSensitivity(SENSITIVITY_128X);
  sensor.setInterruptEnabled();
}

#endif
