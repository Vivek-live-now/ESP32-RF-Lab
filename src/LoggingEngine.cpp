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
    Serial.println("timestamp_ms,rssi_dbm,channel,latency_ms,packet_loss_pct,temp_c");
}

uint8_t LoggingEngine::calculateChecksum(const String& data) const {
    uint8_t checksum = 0;
    for (size_t i = 0; i < data.length(); ++i) {
        checksum ^= data[i];
    }
    return checksum;
}

void LoggingEngine::logDataPoint(uint32_t timestamp, int32_t rssi, int32_t channel, float latencyMs, float packetLoss, float throughputMbps, float tempC) {
    if (!isLogging) return;

    if (format == LogFormat::TELEMETRY) {
        // Format: DATA,seq,timestamp,rssi,channel,latency,loss,temp,checksum
        char buffer[160];
        snprintf(buffer, sizeof(buffer), "DATA,%lu,%lu,%ld,%ld,%.2f,%.2f,%.1f",
                 static_cast<unsigned long>(telemetrySequence++),
                 static_cast<unsigned long>(timestamp),
                 static_cast<long>(rssi),
                 static_cast<long>(channel),
                 latencyMs, packetLoss, tempC);

        String dataStr(buffer);
        uint8_t checksum = calculateChecksum(dataStr);
        Serial.printf("%s,%02X\n", buffer, checksum);
    }
    else if (format == LogFormat::CSV) {
        Serial.printf("%lu,%ld,%ld,%.2f,%.2f,%.1f\n",
            static_cast<unsigned long>(timestamp),
            static_cast<long>(rssi),
            static_cast<long>(channel),
            latencyMs, packetLoss, tempC);
    }
    else if (format == LogFormat::JSON) {
        Serial.printf("  {\"timestamp_ms\": %lu, \"rssi_dbm\": %ld, \"channel\": %ld, \"latency_ms\": %.2f, \"packet_loss_pct\": %.2f, \"temp_c\": %.1f},\n",
            static_cast<unsigned long>(timestamp),
            static_cast<long>(rssi),
            static_cast<long>(channel),
            latencyMs, packetLoss, tempC);
    }
    else {
        // Human format
        Serial.printf("[%lu ms] RSSI: %ld dBm | Ch: %ld | Latency: %.2f ms | Loss: %.2f%% | Temp: %.1f C\n",
            static_cast<unsigned long>(timestamp),
            static_cast<long>(rssi),
            static_cast<long>(channel),
            latencyMs, packetLoss, tempC);
    }
}

