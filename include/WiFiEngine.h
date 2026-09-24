#pragma once

#if defined(ARDUINO)
#include <Arduino.h>
#include <WiFi.h>
#else
#include <string>
#include <cstdint>
using String = std::string;
struct IPAddress {
    uint8_t bytes[4] = {0, 0, 0, 0};
    IPAddress() = default;
    IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
        bytes[0] = a; bytes[1] = b; bytes[2] = c; bytes[3] = d;
    }
    std::string toString() const {
        return std::to_string(bytes[0]) + "." + std::to_string(bytes[1]) + "." +
               std::to_string(bytes[2]) + "." + std::to_string(bytes[3]);
    }
    bool operator==(const IPAddress& o) const {
        return bytes[0] == o.bytes[0] && bytes[1] == o.bytes[1] &&
               bytes[2] == o.bytes[2] && bytes[3] == o.bytes[3];
    }
    bool operator!=(const IPAddress& o) const { return !(*this == o); }
};
#endif
#include <vector>

struct WiFiNetwork {
    String ssid;
    String bssid;
    int32_t rssi;
    int32_t channel;
    uint8_t encryptionType;
};

class WiFiEngine {
public:
    WiFiEngine();
    virtual ~WiFiEngine() = default;

    // Initialize Wi-Fi in Station mode
    virtual void begin();

    // Connect to an AP
    virtual bool connect(const char* ssid, const char* password, uint32_t timeoutMs = 10000);

    // Disconnect from AP
    virtual void disconnect();

    // Check if connected
    virtual bool isConnected() const;

    // Get current RSSI if connected
    virtual int32_t getCurrentRSSI() const;

    // Get current Wi-Fi channel
    virtual int32_t getCurrentChannel() const;

    // Get default Gateway IP
    virtual IPAddress getGatewayIP() const;

    // Get local IP
    virtual IPAddress getLocalIP() const;

    // Perform a Wi-Fi scan and return results
    virtual std::vector<WiFiNetwork> scanNetworks();

    // Print scan results to Serial
    virtual void printScanResults(const std::vector<WiFiNetwork>& networks) const;
};

#if !defined(ARDUINO)
inline WiFiEngine::WiFiEngine() {}
inline void WiFiEngine::begin() {}
inline bool WiFiEngine::connect(const char*, const char*, uint32_t) { return false; }
inline void WiFiEngine::disconnect() {}
inline bool WiFiEngine::isConnected() const { return false; }
inline int32_t WiFiEngine::getCurrentRSSI() const { return 0; }
inline int32_t WiFiEngine::getCurrentChannel() const { return 0; }
inline IPAddress WiFiEngine::getGatewayIP() const { return IPAddress(); }
inline IPAddress WiFiEngine::getLocalIP() const { return IPAddress(); }
inline std::vector<WiFiNetwork> WiFiEngine::scanNetworks() { return {}; }
inline void WiFiEngine::printScanResults(const std::vector<WiFiNetwork>&) const {}
#endif


