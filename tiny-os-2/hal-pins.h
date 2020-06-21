#ifndef _GUARD_H_PINS
#define _GUARD_H_PINS

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

#include <ESP32Encoder.h>
#include <SparkFun_CAP1203_Registers.h>
#include <SparkFun_CAP1203_Types.h>
#include <Wire.h>

ESP32Encoder hw_encoder;
CAP1203 hw_cap_sensor;



#endif