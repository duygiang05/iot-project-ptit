#include <Arduino.h>
#include "PinConfig.h"

// STARTER ONLY: mock telemetry over Serial. No sensor/motor/GPS/MQTT control yet.
// Module owners should add .h/.cpp implementations and submit PRs.
unsigned long lastReport = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Smart Cane firmware starter - MOCK ONLY");
}

void loop() {
  if (millis() - lastReport >= 2000) {
    lastReport = millis();
    Serial.println("{\"device_id\":\"cane-01\",\"distance_cm\":null,\"gps_valid\":false,\"latitude\":null,\"longitude\":null,\"water_detected\":false}");
  }
}
