#if defined(ARDUINO)
#include <Arduino.h>
#endif
#include <unity.h>
#include "MeasurementEngine.h"
#include "../src/MeasurementEngine.cpp"

// Mock for testing
class MockWiFiEngine : public WiFiEngine {
public:
    bool connected = true;
    int32_t mockRssi = -50;
    int32_t mockChannel = 6;

    bool isConnected() const override { return connected; }
    int32_t getCurrentRSSI() const override { return mockRssi; }
    int32_t getCurrentChannel() const override { return mockChannel; }
};

MockWiFiEngine mockWifi;
MeasurementEngine meas(&mockWifi);

void setUp(void) {
    meas.reset();
}

void tearDown(void) {
}

void test_rssi_stats_empty(void) {
    RssiStats stats = meas.getRssiStats();
    TEST_ASSERT_EQUAL(0, stats.samples);
}

void test_rssi_stats_single(void) {
    mockWifi.mockRssi = -60;
    meas.sampleRssi();

    RssiStats stats = meas.getRssiStats();
    TEST_ASSERT_EQUAL(1, stats.samples);
    TEST_ASSERT_EQUAL(-60, stats.min);
    TEST_ASSERT_EQUAL(-60, stats.max);
    TEST_ASSERT_FLOAT_WITHIN(0.01, -60.0, stats.average);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.0, stats.stddev);
}

void test_rssi_stats_multiple(void) {
    mockWifi.mockRssi = -50;
    meas.sampleRssi();
    mockWifi.mockRssi = -60;
    meas.sampleRssi();
    mockWifi.mockRssi = -70;
    meas.sampleRssi();

    RssiStats stats = meas.getRssiStats();
    TEST_ASSERT_EQUAL(3, stats.samples);
    TEST_ASSERT_EQUAL(-70, stats.min);
    TEST_ASSERT_EQUAL(-50, stats.max);
    TEST_ASSERT_FLOAT_WITHIN(0.01, -60.0, stats.average);
    // variance = (100 + 0 + 100) / 3 = 66.66...
    // stddev = sqrt(66.66...) = 8.16...
    TEST_ASSERT_FLOAT_WITHIN(0.1, 8.16, stats.stddev);
}

#if defined(ARDUINO)
void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_rssi_stats_empty);
    RUN_TEST(test_rssi_stats_single);
    RUN_TEST(test_rssi_stats_multiple);
    UNITY_END();
}

void loop() {
}
#else
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_rssi_stats_empty);
    RUN_TEST(test_rssi_stats_single);
    RUN_TEST(test_rssi_stats_multiple);
    return UNITY_END();
}
#endif

