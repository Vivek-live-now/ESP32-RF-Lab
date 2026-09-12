#include "AntennaBenchmarkEngine.h"

AntennaBenchmarkEngine::AntennaBenchmarkEngine(MeasurementEngine* meas) : measurementEngine(meas) {
    antennaAResult.valid = false;
    antennaBResult.valid = false;
}

void AntennaBenchmarkEngine::testAntennaA(uint32_t durationMs) {
    Serial.println("\n--- Testing Antenna A ---");
    Serial.println("Waiting 2s for connection to settle...");
    delay(2000); // settling period

    measurementEngine->reset();
    Serial.println("Collecting samples...");
    measurementEngine->sampleRssiDuration(durationMs);

    antennaAResult.stats = measurementEngine->getRssiStats();
    antennaAResult.valid = true;

    Serial.println("Antenna A testing complete.");
    measurementEngine->printStats();
}

void AntennaBenchmarkEngine::testAntennaB(uint32_t durationMs) {
    Serial.println("\n--- Testing Antenna B ---");
    Serial.println("Waiting 2s for connection to settle...");
    delay(2000); // settling period

    measurementEngine->reset();
    Serial.println("Collecting samples...");
    measurementEngine->sampleRssiDuration(durationMs);

    antennaBResult.stats = measurementEngine->getRssiStats();
    antennaBResult.valid = true;

    Serial.println("Antenna B testing complete.");
    measurementEngine->printStats();
}

void AntennaBenchmarkEngine::compare() const {
    if (!antennaAResult.valid || !antennaBResult.valid) {
        Serial.println("Cannot compare: Test both Antenna A and Antenna B first.");
        return;
    }

    Serial.println("\n=== Antenna Comparison ===");
    Serial.println("Metric        | Antenna A | Antenna B | Difference (B - A)");
    Serial.println("--------------|-----------|-----------|-------------------");

    float avgDiff = antennaBResult.stats.average - antennaAResult.stats.average;
    Serial.printf("Avg RSSI (dBm)| %9.2f | %9.2f | %+9.2f\n",
        antennaAResult.stats.average, antennaBResult.stats.average, avgDiff);

    int minDiff = antennaBResult.stats.min - antennaAResult.stats.min;
    Serial.printf("Min RSSI (dBm)| %9d | %9d | %+9d\n",
        antennaAResult.stats.min, antennaBResult.stats.min, minDiff);

    int maxDiff = antennaBResult.stats.max - antennaAResult.stats.max;
    Serial.printf("Max RSSI (dBm)| %9d | %9d | %+9d\n",
        antennaAResult.stats.max, antennaBResult.stats.max, maxDiff);

    float stddevDiff = antennaBResult.stats.stddev - antennaAResult.stats.stddev;
    Serial.printf("StdDev        | %9.2f | %9.2f | %+9.2f\n",
        antennaAResult.stats.stddev, antennaBResult.stats.stddev, stddevDiff);

    Serial.println("==========================");
    Serial.println("Note: A less negative (closer to 0) RSSI is a stronger signal.");
}
