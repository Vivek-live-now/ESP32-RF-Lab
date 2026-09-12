#include "SerialCLI.h"

SerialCLI::SerialCLI(HardwareAbstraction* h, WiFiEngine* w, MeasurementEngine* m, LoggingEngine* l, AntennaBenchmarkEngine* b)
    : hw(h), wifi(w), meas(m), log(l), bench(b) {
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
        wifi->printScanResults(nets);
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
    // Expected format: CONNECT SSID PASSWORD
    int firstSpace = cmd.indexOf(' ');
    if (firstSpace == -1) return;

    int secondSpace = cmd.indexOf(' ', firstSpace + 1);

    String ssid;
    String pass = "";

    if (secondSpace == -1) {
        // No password
        ssid = cmd.substring(firstSpace + 1);
    } else {
        ssid = cmd.substring(firstSpace + 1, secondSpace);
        pass = cmd.substring(secondSpace + 1);
    }

    ssid.trim();
    pass.trim();

    wifi->connect(ssid.c_str(), pass.c_str());
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
    Serial.println("LOG START   - Start CSV logging");
    Serial.println("LOG STOP    - Stop CSV logging");
    Serial.println("========================");
}
