#include <Arduino.h>
#include "HardwareAbstraction.h"
#include "WiFiEngine.h"
#include "MeasurementEngine.h"
#include "LoggingEngine.h"
#include "AntennaBenchmarkEngine.h"
#include "SerialCLI.h"

// Global instances
HardwareAbstraction hw;
WiFiEngine wifiEngine;
MeasurementEngine measEngine(&wifiEngine);
LoggingEngine logEngine;
AntennaBenchmarkEngine benchEngine(&measEngine);
SerialCLI myCli(&hw, &wifiEngine, &measEngine, &logEngine, &benchEngine);

unsigned long lastLogTime = 0;

void setup() {
    Serial.begin(115200);
    delay(1000); // Give serial time to connect

    Serial.println("\n\n--- ESP32 RF Lab Starting ---");
    hw.printHardwareInfo();

    wifiEngine.begin();

    Serial.println("Type HELP for a list of commands.");
    Serial.print("> ");
}

void loop() {
    myCli.update();

    // Background logging if active
    if (logEngine.isActive() && wifiEngine.isConnected()) {
        uint32_t intervalMs = 1000; // default 1Hz

        if (logEngine.getFormat() == LogFormat::TELEMETRY) {
            uint32_t rate = myCli.getTelemetryRate();
            if (rate > 0) {
                intervalMs = 1000 / rate;
            }
        }

        if (millis() - lastLogTime >= intervalMs) {
            lastLogTime = millis();
            int32_t rssi = wifiEngine.getCurrentRSSI();
            // In a fuller implementation, latency, loss, and throughput would be measured here.
            logEngine.logDataPoint(millis(), rssi, 0, 0.0f, 0.0f, 0.0f);
        }
    }
}
