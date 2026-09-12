#include "HardwareAbstraction.h"
#include <esp_system.h>
#if defined(ESP32)
#include <esp_chip_info.h>
#endif

HardwareAbstraction::HardwareAbstraction() {
}

String HardwareAbstraction::getChipModel() const {
#if defined(CONFIG_IDF_TARGET_ESP32)
    return "ESP32";
#elif defined(CONFIG_IDF_TARGET_ESP32S2)
    return "ESP32-S2";
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
    return "ESP32-S3";
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
    return "ESP32-C3";
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
    return "ESP32-C6";
#else
    return "Unknown ESP32 variant";
#endif
}

uint8_t HardwareAbstraction::getChipRevision() const {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    return chip_info.revision;
}

uint8_t HardwareAbstraction::getCpuCores() const {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    return chip_info.cores;
}

String HardwareAbstraction::getSdkVersion() const {
    return String(ESP.getSdkVersion());
}

void HardwareAbstraction::printHardwareInfo() const {
    Serial.println("=== Hardware Information ===");
    Serial.print("Model: ");
    Serial.println(getChipModel());
    Serial.print("Revision: ");
    Serial.println(getChipRevision());
    Serial.print("Cores: ");
    Serial.println(getCpuCores());
    Serial.print("SDK Version: ");
    Serial.println(getSdkVersion());
    Serial.println("============================");
}
