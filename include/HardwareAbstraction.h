#pragma once
#include <Arduino.h>

class HardwareAbstraction {
public:
    HardwareAbstraction();

    // Get chip model (ESP32, ESP32-S2, etc.)
    String getChipModel() const;

    // Get chip revision
    uint8_t getChipRevision() const;

    // Get number of CPU cores
    uint8_t getCpuCores() const;

    // Get SDK/Core version
    String getSdkVersion() const;

    // Get internal chip temperature in Celsius
    float getTemperatureC() const;

    // Print hardware info to Serial
    void printHardwareInfo() const;
};

