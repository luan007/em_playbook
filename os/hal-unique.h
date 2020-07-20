#define OS_CORE_VERSION "1.0.0"
char esp_chip_id[20];
char *get_chip_id()
{
    uint64_t chipid = ESP.getEfuseMac();
    sprintf(esp_chip_id, "%04X%04X%04X", (uint16_t)(chipid >> 32), (uint16_t)(chipid >> 16), (uint16_t)chipid);
}
