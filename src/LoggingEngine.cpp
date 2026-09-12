#include "LoggingEngine.h"

LoggingEngine::LoggingEngine() : isLogging(false), format(LogFormat::CSV) {
}

void LoggingEngine::startLog(LogFormat fmt) {
    format = fmt;
    isLogging = true;
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

void LoggingEngine::printHeader() {
    Serial.println("timestamp_ms,rssi_dbm,channel,latency_ms,packet_loss_pct,throughput_mbps");
}

void LoggingEngine::logDataPoint(uint32_t timestamp, int32_t rssi, int32_t channel, float latencyMs, float packetLoss, float throughputMbps) {
    if (!isLogging) return;

    if (format == LogFormat::CSV) {
        Serial.printf("%lu,%d,%d,%.2f,%.2f,%.2f\n",
            timestamp, rssi, channel, latencyMs, packetLoss, throughputMbps);
    } else if (format == LogFormat::JSON) {
        Serial.printf("  {\"timestamp_ms\": %lu, \"rssi_dbm\": %d, \"channel\": %d, \"latency_ms\": %.2f, \"packet_loss_pct\": %.2f, \"throughput_mbps\": %.2f},\n",
            timestamp, rssi, channel, latencyMs, packetLoss, throughputMbps);
    } else {
        // Human format
        Serial.printf("[%lu ms] RSSI: %d dBm | Ch: %d | Latency: %.2f ms | Loss: %.2f%% | Throughput: %.2f Mbps\n",
            timestamp, rssi, channel, latencyMs, packetLoss, throughputMbps);
    }
}
