#include "MeasurementEngine.h"

MeasurementEngine::MeasurementEngine(WiFiEngine* wifi) : wifiEngine(wifi) {
}

void MeasurementEngine::reset() {
    rssiSamples.clear();
}

void MeasurementEngine::sampleRssi() {
    if (wifiEngine->isConnected()) {
        int32_t rssi = wifiEngine->getCurrentRSSI();
        rssiSamples.push_back(rssi);
    }
}

void MeasurementEngine::sampleRssiDuration(uint32_t durationMs, uint32_t intervalMs) {
    uint32_t start = millis();
    while (millis() - start < durationMs) {
        sampleRssi();
        delay(intervalMs);
    }
}

RssiStats MeasurementEngine::getRssiStats() const {
    RssiStats stats = {0, 0, 0.0f, 0.0f, 0.0f, 0};
    stats.samples = rssiSamples.size();

    if (stats.samples == 0) return stats;

    stats.min = rssiSamples[0];
    stats.max = rssiSamples[0];
    int64_t sum = 0;

    for (int32_t val : rssiSamples) {
        if (val < stats.min) stats.min = val;
        if (val > stats.max) stats.max = val;
        sum += val;
    }

    stats.average = static_cast<float>(sum) / stats.samples;

    float varianceSum = 0;
    for (int32_t val : rssiSamples) {
        varianceSum += (val - stats.average) * (val - stats.average);
    }

    stats.variance = varianceSum / stats.samples;
    stats.stddev = std::sqrt(stats.variance);

    return stats;
}

void MeasurementEngine::printStats() const {
    RssiStats stats = getRssiStats();
    Serial.println("=== RSSI Statistics ===");
    Serial.printf("Samples: %zu\n", stats.samples);
    if (stats.samples > 0) {
        Serial.printf("Min:     %d dBm\n", stats.min);
        Serial.printf("Max:     %d dBm\n", stats.max);
        Serial.printf("Average: %.2f dBm\n", stats.average);
        Serial.printf("StdDev:  %.2f\n", stats.stddev);
    } else {
        Serial.println("No samples collected.");
    }
    Serial.println("=======================");
}
