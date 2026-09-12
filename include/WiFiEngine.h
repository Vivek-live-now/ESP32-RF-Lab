#pragma once
#include <Arduino.h>
#include <WiFi.h>
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

    // Initialize Wi-Fi in Station mode
    void begin();

    // Connect to an AP
    bool connect(const char* ssid, const char* password, uint32_t timeoutMs = 10000);

    // Disconnect from AP
    void disconnect();

    // Check if connected
    bool isConnected() const;

    // Get current RSSI if connected
    int32_t getCurrentRSSI() const;

    // Perform a Wi-Fi scan and return results
    std::vector<WiFiNetwork> scanNetworks();

    // Print scan results to Serial
    void printScanResults(const std::vector<WiFiNetwork>& networks) const;
};
