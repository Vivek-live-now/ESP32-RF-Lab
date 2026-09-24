#include "SerialCLI.h"

SerialCLI::SerialCLI(HardwareAbstraction* h, WiFiEngine* w, MeasurementEngine* m, LoggingEngine* l, AntennaBenchmarkEngine* b, PingEngine* p)
    : hw(h), wifi(w), meas(m), log(l), bench(b), ping(p), telemetryRateHz(1) {
}

void SerialCLI::update() {
    while (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (commandBuffer.length() > 0) {
                processCommand(commandBuffer);
                commandBuffer = "";
            }
        } else {
            commandBuffer += c;
        }
    }
}

uint32_t SerialCLI::getTelemetryRate() const {
    return telemetryRateHz;
}

void SerialCLI::processCommand(const String& cmdRaw) {
    String cmd = cmdRaw;
    cmd.trim();
    String cmdUpper = cmd;
    cmdUpper.toUpperCase();

    if (cmdUpper == "HELP") {
        printHelp();
    }
    else if (cmdUpper == "INFO") {
        hw->printHardwareInfo();
    }
    else if (cmdUpper == "SCAN") {
        auto nets = wifi->scanNetworks();
        // Print as structured JSON if GUI stream is active, otherwise human
        if (log->isActive() && log->getFormat() == LogFormat::TELEMETRY) {
            Serial.println("SCAN_START");
            for (const auto& net : nets) {
                char buffer[256];
                snprintf(buffer, sizeof(buffer), "SCAN_RES,%s,%s,%ld,%ld,%d",
                    net.ssid.c_str(), net.bssid.c_str(), (long)net.rssi, (long)net.channel, net.encryptionType);
                String dataStr(buffer);
                uint8_t checksum = 0;
                for (size_t i = 0; i < dataStr.length(); ++i) checksum ^= dataStr[i];
                Serial.printf("%s,%02X\n", buffer, checksum);
            }
            Serial.println("SCAN_END");
        } else {
            wifi->printScanResults(nets);
        }
    }
    else if (cmdUpper.startsWith("CONNECT ")) {
        handleConnect(cmd);
    }
    else if (cmdUpper == "DISCONNECT") {
        wifi->disconnect();
    }
    else if (cmdUpper == "STATUS") {
        if (wifi->isConnected()) {
            Serial.println("=== Connection Status ===");
            Serial.println("State:    Connected");
            Serial.printf("SSID:     %s\n", WiFi.SSID().c_str());
            Serial.printf("IP:       %s\n", wifi->getLocalIP().toString().c_str());
            Serial.printf("Gateway:  %s\n", wifi->getGatewayIP().toString().c_str());
            Serial.printf("Channel:  %ld\n", (long)wifi->getCurrentChannel());
            Serial.printf("RSSI:     %ld dBm\n", (long)wifi->getCurrentRSSI());
            Serial.printf("Temp:     %.1f °C\n", hw->getTemperatureC());
            Serial.println("=========================");
        } else {
            Serial.println("State: Disconnected");
        }
    }
    else if (cmdUpper == "RSSI") {
        if (wifi->isConnected()) {
            Serial.printf("Current RSSI: %ld dBm (Channel %ld)\n",
                (long)wifi->getCurrentRSSI(), (long)wifi->getCurrentChannel());
        } else {
            Serial.println("Error: Not connected to Wi-Fi");
        }
    }
    else if (cmdUpper.startsWith("ANTENNA A")) {
        if (!wifi->isConnected()) {
            Serial.println("Error: Must be connected to Wi-Fi first.");
            return;
        }
        bench->testAntennaA(10000); // 10 seconds test
    }
    else if (cmdUpper.startsWith("ANTENNA B")) {
        if (!wifi->isConnected()) {
            Serial.println("Error: Must be connected to Wi-Fi first.");
            return;
        }
        bench->testAntennaB(10000); // 10 seconds test
    }
    else if (cmdUpper == "COMPARE") {
        bench->compare();
    }
    else if (cmdUpper.startsWith("STREAM START")) {
        handleStream(cmdUpper);
    }
    else if (cmdUpper == "STREAM STOP") {
        log->stopLog();
        Serial.println("ACK_STREAM_STOP");
    }
    else if (cmdUpper == "LOG START") {
        log->startLog(LogFormat::CSV);
        Serial.println("Started CSV logging.");
    }
    else if (cmdUpper == "LOG STOP") {
        log->stopLog();
        Serial.println("Stopped logging.");
    }
    else if (cmdUpper.startsWith("PING")) {
        handlePing(cmd);
    }
    else if (cmdUpper.startsWith("STABILITY")) {
        handleStability(cmd);
    }
    else if (cmdUpper == "THROUGHPUT") {
        Serial.printf("Channel: %ld | Current RSSI: %ld dBm\n",
            (long)wifi->getCurrentChannel(), (long)wifi->getCurrentRSSI());
        Serial.println("Theoretical 802.11n PHY max rate: 72.2 Mbps (HT20) / 150 Mbps (HT40).");
        Serial.println("Actual TCP throughput on ESP32 reaches ~20-30 Mbps.");
    }
    else {
        Serial.println("Unknown command. Type HELP for a list of commands.");
    }
}

void SerialCLI::handleConnect(const String& cmd) {
    int firstSpace = cmd.indexOf(' ');
    if (firstSpace == -1) return;

    int secondSpace = cmd.indexOf(' ', firstSpace + 1);

    String ssid;
    String pass = "";

    if (secondSpace == -1) {
        ssid = cmd.substring(firstSpace + 1);
    } else {
        ssid = cmd.substring(firstSpace + 1, secondSpace);
        pass = cmd.substring(secondSpace + 1);
    }

    ssid.trim();
    pass.trim();

    wifi->connect(ssid.c_str(), pass.c_str());
    if (log->isActive() && log->getFormat() == LogFormat::TELEMETRY) {
        Serial.println("ACK_CONNECT");
    }
}

void SerialCLI::handleStream(const String& cmdUpper) {
    int rate = 1; // default 1Hz
    int lastSpace = cmdUpper.lastIndexOf(' ');

    if (lastSpace > 0 && lastSpace != cmdUpper.indexOf(' ')) {
        String rateStr = cmdUpper.substring(lastSpace + 1);
        rate = rateStr.toInt();
        if (rate <= 0) rate = 1;
        if (rate > 20) rate = 20; // Cap at 20Hz
    }

    telemetryRateHz = rate;
    log->startLog(LogFormat::TELEMETRY);
    Serial.printf("ACK_STREAM_START,%lu\n", (unsigned long)telemetryRateHz);
}

void SerialCLI::handlePing(const String& cmd) {
    if (!wifi->isConnected()) {
        Serial.println("Error: Must be connected to Wi-Fi first.");
        return;
    }
    if (!ping) {
        Serial.println("Error: PingEngine not available.");
        return;
    }

    String target = "";
    int spaceIdx = cmd.indexOf(' ');
    if (spaceIdx != -1) {
        target = cmd.substring(spaceIdx + 1);
        target.trim();
    }

    IPAddress targetIp;
    if (target.length() == 0) {
        targetIp = wifi->getGatewayIP();
        if (targetIp == IPAddress(0, 0, 0, 0)) {
            Serial.println("Error: Default gateway unavailable.");
            return;
        }
    } else {
        if (!targetIp.fromString(target)) {
            if (!WiFi.hostByName(target.c_str(), targetIp)) {
                Serial.printf("Error: Could not resolve '%s'\n", target.c_str());
                return;
            }
        }
    }

    Serial.printf("PING %s (4 packets)...\n", targetIp.toString().c_str());
    ping->pingHost(targetIp, 4, 1000);
    const auto& stats = ping->getLastStats();

    Serial.printf("--- %s ping statistics ---\n", targetIp.toString().c_str());
    Serial.printf("%lu packets transmitted, %lu received, %.1f%% packet loss\n",
        (unsigned long)stats.sent, (unsigned long)stats.received, stats.packetLossPct);
    if (stats.received > 0) {
        Serial.printf("rtt min/avg/max = %.2f/%.2f/%.2f ms\n",
            stats.minLatencyMs, stats.avgLatencyMs, stats.maxLatencyMs);
    }
    Serial.printf("PING_RES,%s,%lu,%lu,%.1f,%.2f\n",
        targetIp.toString().c_str(),
        (unsigned long)stats.sent,
        (unsigned long)stats.received,
        stats.packetLossPct,
        stats.avgLatencyMs);
}

void SerialCLI::handleStability(const String& cmd) {
    if (!wifi->isConnected()) {
        Serial.println("Error: Must be connected to Wi-Fi first.");
        return;
    }

    uint32_t durationSec = 10;
    int spaceIdx = cmd.indexOf(' ');
    if (spaceIdx != -1) {
        int val = cmd.substring(spaceIdx + 1).toInt();
        if (val >= 3 && val <= 60) durationSec = val;
    }

    Serial.printf("--- Link Stability Test (%lu seconds) ---\n", (unsigned long)durationSec);
    meas->reset();

    uint32_t startMs = millis();
    uint32_t durationMs = durationSec * 1000;
    IPAddress gw = wifi->getGatewayIP();

    while (millis() - startMs < durationMs) {
        meas->sampleRssi();
        delay(250);
    }

    RssiStats rStats = meas->getRssiStats();
    Serial.println("=== Stability Results ===");
    Serial.printf("RSSI Samples:  %zu\n", rStats.samples);
    Serial.printf("RSSI Mean:     %.2f dBm\n", rStats.average);
    Serial.printf("RSSI Min/Max:  %ld / %ld dBm\n", (long)rStats.min, (long)rStats.max);
    Serial.printf("RSSI Jitter:   %.2f dB\n", rStats.stddev);

    if (ping && gw != IPAddress(0, 0, 0, 0)) {
        ping->pingHost(gw, 4, 800);
        const auto& pStats = ping->getLastStats();
        Serial.printf("Gateway Ping:  %.2f ms (Loss: %.1f%%)\n", pStats.avgLatencyMs, pStats.packetLossPct);
    }

    float stabilityScore = 100.0f;
    if (rStats.stddev > 3.0f) stabilityScore -= (rStats.stddev - 3.0f) * 5.0f;
    if (rStats.average < -75.0f) stabilityScore -= (-75.0f - rStats.average) * 2.0f;
    if (ping) stabilityScore -= ping->getLastPacketLoss() * 0.5f;
    if (stabilityScore < 0.0f) stabilityScore = 0.0f;
    if (stabilityScore > 100.0f) stabilityScore = 100.0f;

    Serial.printf("Link Health:   %.1f / 100\n", stabilityScore);
    Serial.println("=========================");
}

void SerialCLI::printHelp() const {
    Serial.println("=== ESP32 RF Lab CLI ===");
    Serial.println("HELP                  - Show this help");
    Serial.println("INFO                  - Show hardware info and chip temperature");
    Serial.println("SCAN                  - Scan for Wi-Fi networks");
    Serial.println("CONNECT <ssid> [pass] - Connect to network");
    Serial.println("DISCONNECT            - Disconnect from network");
    Serial.println("STATUS                - Show connection, IP, channel, and RSSI");
    Serial.println("RSSI                  - Show current RSSI and channel");
    Serial.println("PING [ip/host]        - Ping gateway or host (latency & packet loss)");
    Serial.println("STABILITY [sec]       - Assess RSSI jitter & link health score");
    Serial.println("ANTENNA A             - Run 10s benchmark for Antenna A");
    Serial.println("ANTENNA B             - Run 10s benchmark for Antenna B");
    Serial.println("COMPARE               - Compare A/B benchmark results");
    Serial.println("STREAM START [rate]   - Start GUI telemetry (1, 2, 5, 10 Hz)");
    Serial.println("STREAM STOP           - Stop GUI telemetry");
    Serial.println("LOG START             - Start CSV logging");
    Serial.println("LOG STOP              - Stop CSV logging");
    Serial.println("THROUGHPUT            - Show link theoretical and actual limits");
    Serial.println("========================");
}

