during building

Modify size

esp32.upload.maximum_size=8388608

use following partition table

# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x5000,
otadata,  data, ota,     0xe000,  0x2000,
app0,     app,  ota_0,   0x10000, 0x800000,
spiffs,   data, spiffs,  0x910000,0x6F0000,

AKA:
~/Library/Arduino15/packages/esp32/hardware/esp32/1.0.4/tools/partitions
DEFAULT 16