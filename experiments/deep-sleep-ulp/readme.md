test field for ULP + esp32


## References
-> 
https://github.com/duff2013/ulptool

-> 
https://github.com/SensorsIot/ESP32-ULP-Arduino-IDE/tree/master/ulp_adc


## Note..

DO NOT MODIFY ULP_COUNTER.C (like, adding comments up top / below, as it will break the compiler) hope this gets fixed soon.

https://github.com/duff2013/ulptool/issues/50


## Testing

with out ESP32 board (FTDI only)
`22.89mA`

with absolutely no logic (sleep only)
`24.09mA`

with ULP running while true loop - calculated delta 0.02mA 
`24.11mA`

with ESP32 Firmware uploader (bootloader)
`37.64mA`


PCB drain: ~`1.2mA` - need to find out why


## Test2
FTDI `22.89` benchmark
BOOT `35.49`
ULP LOOP `23.28` delta `0.39mA` always on


## 0.2mA 
1000maH -> 60d ~ 100d operation on battery

## Test 3 with only esp32 + cp2102
CP2102  `16.51`
ULP LOOP `16.58` DELTA `0.07mA`


## thus.. possible leak in schematics - TBC


## 

Download: 1min - 3xTime / day - 200mA = 10mAh / day
Interaction: 3min = 0.05 x 100mA = 5mAh / day
Idle: 0.2mA ~ 24H = 4.8mAh / day

Day 20mAH = 50 ~ 150d life