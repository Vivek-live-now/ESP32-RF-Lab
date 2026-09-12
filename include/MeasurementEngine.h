#pragma once
#include <Arduino.h>
#include <vector>
#include <numeric>
#include <cmath>
#include "WiFiEngine.h"

struct RssiStats {
    int32_t min;
    int32_t max;
    float average;
    float variance;
    float stddev;
    size_t samples;
};

class MeasurementEngine {
private:
    WiFiEngine* wifiEngine;
    std::vector<int32_t> rssiSamples;

public:
    MeasurementEngine(WiFiEngine* wifi);

    // Clear collected samples
    void reset();

    // Collect a single RSSI sample
    void sampleRssi();

    // Collect N samples over a given duration
    void sampleRssiDuration(uint32_t durationMs, uint32_t intervalMs = 100);

    // Calculate and return stats
    RssiStats getRssiStats() const;

    // Print stats to Serial
    void printStats() const;

    // Perform a basic latency test (ping approximation using HTTP/TCP to a known fast host could be added,
    // here we might mock or implement a simple ICMP ping later)
    // float measureLatency(const char* host);
};
