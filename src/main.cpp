#include <Arduino.h>
#include "HardwareAbstraction.h"
#include "WiFiEngine.h"
#include "MeasurementEngine.h"
#include "LoggingEngine.h"
#include "AntennaBenchmarkEngine.h"
#include "PingEngine.h"
#include "SerialCLI.h"

// Global instances
HardwareAbstraction hw;
WiFiEngine wifiEngine;
MeasurementEngine measEngine(&wifiEngine);
LoggingEngine logEngine;
AntennaBenchmarkEngine benchEngine(&measEngine);
PingEngine pingEngine;
SerialCLI myCli(&hw, &wifiEngine, &measEngine, &logEngine, &benchEngine, &pingEngine);

unsigned long lastLogTime = 0;
unsigned long lastPingTime = 0;

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

        // Periodically refresh gateway ping (every 4 seconds) to update latency & loss
        if (millis() - lastPingTime >= 4000) {
            lastPingTime = millis();
            IPAddress gw = wifiEngine.getGatewayIP();
            if (gw != IPAddress(0, 0, 0, 0)) {
                pingEngine.pingHost(gw, 1, 300);
            }
        }

        if (millis() - lastLogTime >= intervalMs) {
            lastLogTime = millis();
            int32_t rssi = wifiEngine.getCurrentRSSI();
            int32_t channel = wifiEngine.getCurrentChannel();
            float latency = pingEngine.getLastLatency();
            float loss = pingEngine.getLastPacketLoss();
            float temp = hw.getTemperatureC();

            logEngine.logDataPoint(millis(), rssi, channel, latency, loss, 0.0f, temp);
        }
    }
}

