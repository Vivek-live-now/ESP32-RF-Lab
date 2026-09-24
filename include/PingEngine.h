#pragma once
#include <Arduino.h>
#include <IPAddress.h>

struct PingStats {
    uint32_t sent;
    uint32_t received;
    float minLatencyMs;
    float maxLatencyMs;
    float avgLatencyMs;
    float packetLossPct;

    PingStats() : sent(0), received(0), minLatencyMs(0.0f), maxLatencyMs(0.0f), avgLatencyMs(0.0f), packetLossPct(0.0f) {}
};

class PingEngine {
private:
    PingStats lastStats;

public:
    PingEngine();
    virtual ~PingEngine() = default;

    virtual bool pingHost(IPAddress ip, uint32_t count = 3, uint32_t timeoutMs = 1000);
    virtual bool pingHost(const char* host, uint32_t count = 3, uint32_t timeoutMs = 1000);

    const PingStats& getLastStats() const { return lastStats; }
    float getLastLatency() const { return lastStats.avgLatencyMs; }
    float getLastPacketLoss() const { return lastStats.packetLossPct; }
};
