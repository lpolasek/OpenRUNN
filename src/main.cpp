#include <Arduino.h>

#include "TCRT5000.h"
#include "BLE_RSC.h"

const uint8_t SENSOR_PIN = 4;          // TCRT5000 output

const unsigned long RSC_NOTIFICATION_INTERVAL_MS = 1000;

const float beltLengthMeters = 3.19f;        // Distance per valid sensor pulse
const float MAX_SPEED_KPH = 25.0f;
const unsigned long MIN_STOP_TIMEOUT_MS = 750;
const unsigned long MAX_STOP_TIMEOUT_MS = 5000;

unsigned long lastRscNotificationMs = 0;

unsigned long sensorDebounceMs() {
  float maxSpeedMetersPerSecond = MAX_SPEED_KPH / 3.6f;
  return (unsigned long)((beltLengthMeters * 0.5f * 1000.0f) / maxSpeedMetersPerSecond);
}

float getSpeedMetersPerSecond(const TCRT5000Data& data) {
    unsigned long pulseIntervalMs = data.lastPulseIntervalMs;

    unsigned long stopTimeoutMs = pulseIntervalMs > 0 ? pulseIntervalMs * 1.5 : MIN_STOP_TIMEOUT_MS;
    stopTimeoutMs = max(MIN_STOP_TIMEOUT_MS, min(stopTimeoutMs, MAX_STOP_TIMEOUT_MS));
    bool moving = data.hasPulse && (millis() - data.lastPulseMs) < stopTimeoutMs;
    float speedMetersPerSecond = moving && pulseIntervalMs > 0
      ? beltLengthMeters * 1000.0f / pulseIntervalMs
      : 0.0f;
    return min(speedMetersPerSecond, MAX_SPEED_KPH / 3.6f);
}

TCRT5000 sensor(SENSOR_PIN, sensorDebounceMs());
BLE_RSC bleRsc("OpenRUNN");

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Starting ESP32 Run Sensor...");

  sensor.setup();
  bleRsc.setup();
}

void loop() {
  unsigned long nowMs = millis();
  TCRT5000Data data = sensor.getData();

  if (bleRsc.isConnected() && (nowMs - lastRscNotificationMs) >= RSC_NOTIFICATION_INTERVAL_MS) {
    lastRscNotificationMs = nowMs;
    float speedMetersPerSecond = getSpeedMetersPerSecond(data);
    float totalDistanceMeters = data.totalPulses * beltLengthMeters;

    bleRsc.notifyRscMeasurement(speedMetersPerSecond, 0, totalDistanceMeters);
  }
}
