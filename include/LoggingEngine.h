#pragma once
#include <Arduino.h>

enum class LogFormat {
    CSV,
    JSON,
    HUMAN
};

class LoggingEngine {
private:
    bool isLogging;
    LogFormat format;

public:
    LoggingEngine();

    void startLog(LogFormat fmt = LogFormat::CSV);
    void stopLog();
    bool isActive() const;

    // Log a data point
    void logDataPoint(uint32_t timestamp, int32_t rssi, int32_t channel, float latencyMs, float packetLoss, float throughputMbps);

    // Print CSV header
    void printHeader();
};
