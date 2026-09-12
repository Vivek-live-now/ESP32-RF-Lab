#include "WiFiEngine.h"

WiFiEngine::WiFiEngine() {
}

void WiFiEngine::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    delay(100);
}

bool WiFiEngine::connect(const char* ssid, const char* password, uint32_t timeoutMs) {
    Serial.printf("Connecting to %s...\n", ssid);
    WiFi.begin(ssid, password);

    uint32_t startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeoutMs) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("Connection failed.");
        return false;
    }
}

void WiFiEngine::disconnect() {
    WiFi.disconnect(true);
    Serial.println("Disconnected from Wi-Fi.");
}

bool WiFiEngine::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

int32_t WiFiEngine::getCurrentRSSI() const {
    if (isConnected()) {
        return WiFi.RSSI();
    }
    return 0;
}

std::vector<WiFiNetwork> WiFiEngine::scanNetworks() {
    std::vector<WiFiNetwork> networks;
    Serial.println("Starting network scan...");
    int n = WiFi.scanNetworks();
    Serial.println("Scan complete.");

    if (n == 0) {
        Serial.println("No networks found.");
    } else {
        for (int i = 0; i < n; ++i) {
            WiFiNetwork net;
            net.ssid = WiFi.SSID(i);
            net.bssid = WiFi.BSSIDstr(i);
            net.rssi = WiFi.RSSI(i);
            net.channel = WiFi.channel(i);
            net.encryptionType = WiFi.encryptionType(i);
            networks.push_back(net);
        }
    }

    WiFi.scanDelete();
    return networks;
}

void WiFiEngine::printScanResults(const std::vector<WiFiNetwork>& networks) const {
    Serial.printf("Found %zu networks:\n", networks.size());
    for (size_t i = 0; i < networks.size(); ++i) {
        const auto& net = networks[i];
        Serial.printf("%2zu: %-32s (%d dBm) CH: %2d BSSID: %s\n",
            i + 1, net.ssid.c_str(), net.rssi, net.channel, net.bssid.c_str());
    }
}
