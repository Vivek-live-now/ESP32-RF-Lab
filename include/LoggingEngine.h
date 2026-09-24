#pragma once
#include <Arduino.h>

enum class LogFormat {
    CSV,
    JSON,
    HUMAN,
    TELEMETRY
};

class LoggingEngine {
private:
    bool isLogging;
    LogFormat format;
    uint32_t telemetrySequence;

    uint8_t calculateChecksum(const String& data) const;

public:
    LoggingEngine();

    void startLog(LogFormat fmt = LogFormat::CSV);
    void stopLog();
    bool isActive() const;
    LogFormat getFormat() const;

    // Log a data point
    void logDataPoint(uint32_t timestamp, int32_t rssi, int32_t channel, float latencyMs, float packetLoss, float throughputMbps = 0.0f, float tempC = 0.0f);


    // Print CSV header
    void printHeader();
};
