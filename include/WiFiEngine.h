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

