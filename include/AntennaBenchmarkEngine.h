#pragma once
#include <Arduino.h>
#include "MeasurementEngine.h"

struct BenchmarkResult {
    bool valid;
    RssiStats stats;
    // Add latency, throughput, packet loss here later
};

class AntennaBenchmarkEngine {
private:
    MeasurementEngine* measurementEngine;
    BenchmarkResult antennaAResult;
    BenchmarkResult antennaBResult;

public:
    AntennaBenchmarkEngine(MeasurementEngine* meas);

    // Run test for a specific antenna
    void testAntennaA(uint32_t durationMs);
    void testAntennaB(uint32_t durationMs);

    // Compare results
    void compare() const;
};
