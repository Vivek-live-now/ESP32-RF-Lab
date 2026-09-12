#include "SerialCLI.h"

SerialCLI::SerialCLI(HardwareAbstraction* h, WiFiEngine* w, MeasurementEngine* m, LoggingEngine* l, AntennaBenchmarkEngine* b)
    : hw(h), wifi(w), meas(m), log(l), bench(b), telemetryRateHz(1) {
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
                // Calculate checksum for scan items too
                char buffer[256];
                snprintf(buffer, sizeof(buffer), "SCAN_RES,%s,%s,%d,%d,%d",
                    net.ssid.c_str(), net.bssid.c_str(), net.rssi, net.channel, net.encryptionType);
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
            Serial.println("Status: Connected");
            Serial.print("IP: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("Status: Disconnected");
        }
    }
    else if (cmdUpper == "RSSI") {
        if (wifi->isConnected()) {
            Serial.printf("Current RSSI: %d dBm\n", wifi->getCurrentRSSI());
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
    else if (cmdUpper == "PING" || cmdUpper == "THROUGHPUT" || cmdUpper == "STABILITY") {
        Serial.println("Command acknowledged but not fully implemented in this version.");
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
    // STREAM START [RATE]
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
    Serial.printf("ACK_STREAM_START,%d\n", telemetryRateHz);
}

void SerialCLI::printHelp() const {
    Serial.println("=== ESP32 RF Lab CLI ===");
    Serial.println("HELP        - Show this help");
    Serial.println("INFO        - Show hardware info");
    Serial.println("SCAN        - Scan for Wi-Fi networks");
    Serial.println("CONNECT <ssid> [pass] - Connect to network");
    Serial.println("DISCONNECT  - Disconnect from network");
    Serial.println("STATUS      - Show connection status");
    Serial.println("RSSI        - Show current RSSI");
    Serial.println("ANTENNA A   - Run 10s benchmark for Antenna A");
    Serial.println("ANTENNA B   - Run 10s benchmark for Antenna B");
    Serial.println("COMPARE     - Compare A/B benchmark results");
    Serial.println("STREAM START [rate] - Start GUI telemetry (1, 2, 5, 10 Hz)");
    Serial.println("STREAM STOP - Stop GUI telemetry");
    Serial.println("LOG START   - Start CSV logging");
    Serial.println("LOG STOP    - Stop CSV logging");
    Serial.println("========================");
}
