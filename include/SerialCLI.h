#pragma once
#include <Arduino.h>
#include "HardwareAbstraction.h"
#include "WiFiEngine.h"
#include "MeasurementEngine.h"
#include "LoggingEngine.h"
#include "AntennaBenchmarkEngine.h"

class SerialCLI {
private:
    HardwareAbstraction* hw;
    WiFiEngine* wifi;
    MeasurementEngine* meas;
    LoggingEngine* log;
    AntennaBenchmarkEngine* bench;

    String commandBuffer;

    // Telemetry rate in Hz (0 means off, handled by main loop)
    uint32_t telemetryRateHz;

    void processCommand(const String& cmd);
    void printHelp() const;
    void handleConnect(const String& cmd);
    void handleStream(const String& cmd);

public:
    SerialCLI(HardwareAbstraction* h, WiFiEngine* w, MeasurementEngine* m, LoggingEngine* l, AntennaBenchmarkEngine* b);

    // Poll for serial input
    void update();

    uint32_t getTelemetryRate() const;
};
