#include "LoggingEngine.h"

LoggingEngine::LoggingEngine() : isLogging(false), format(LogFormat::CSV), telemetrySequence(0) {
}

void LoggingEngine::startLog(LogFormat fmt) {
    format = fmt;
    isLogging = true;
    telemetrySequence = 0;

    if (format == LogFormat::CSV) {
        printHeader();
    } else if (format == LogFormat::JSON) {
        Serial.println("[");
    }
}

void LoggingEngine::stopLog() {
    if (!isLogging) return;
    if (format == LogFormat::JSON) {
        Serial.println("]");
    }
    isLogging = false;
}

bool LoggingEngine::isActive() const {
    return isLogging;
}

LogFormat LoggingEngine::getFormat() const {
    return format;
}

void LoggingEngine::printHeader() {
    Serial.println("timestamp_ms,rssi_dbm,channel,latency_ms,packet_loss_pct,throughput_mbps");
}

uint8_t LoggingEngine::calculateChecksum(const String& data) const {
    uint8_t checksum = 0;
    for (size_t i = 0; i < data.length(); ++i) {
        checksum ^= data[i];
    }
    return checksum;
}

void LoggingEngine::logDataPoint(uint32_t timestamp, int32_t rssi, int32_t channel, float latencyMs, float packetLoss, float throughputMbps) {
    if (!isLogging) return;

    if (format == LogFormat::TELEMETRY) {
        // Format: DATA,seq,timestamp,rssi,channel,latency,loss
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "DATA,%lu,%lu,%d,%d,%.2f,%.2f",
                 telemetrySequence++, timestamp, rssi, channel, latencyMs, packetLoss);

        String dataStr(buffer);
        uint8_t checksum = calculateChecksum(dataStr);
        Serial.printf("%s,%02X\n", buffer, checksum);
    }
    else if (format == LogFormat::CSV) {
        Serial.printf("%lu,%d,%d,%.2f,%.2f,%.2f\n",
            timestamp, rssi, channel, latencyMs, packetLoss, throughputMbps);
    }
    else if (format == LogFormat::JSON) {
        Serial.printf("  {\"timestamp_ms\": %lu, \"rssi_dbm\": %d, \"channel\": %d, \"latency_ms\": %.2f, \"packet_loss_pct\": %.2f, \"throughput_mbps\": %.2f},\n",
            timestamp, rssi, channel, latencyMs, packetLoss, throughputMbps);
    }
    else {
        // Human format
        Serial.printf("[%lu ms] RSSI: %d dBm | Ch: %d | Latency: %.2f ms | Loss: %.2f%% | Throughput: %.2f Mbps\n",
            timestamp, rssi, channel, latencyMs, packetLoss, throughputMbps);
    }
}
