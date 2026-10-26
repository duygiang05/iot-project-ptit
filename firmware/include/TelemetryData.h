#pragma once
// Shared interface for independently developed sensor modules.
// In G06 all fields are synthetic placeholders; replace with real modules during G09.
struct TelemetryData {
  float distanceCm = 45.0f;
  bool gpsValid = false;
  bool waterDetected = false;
  int lightLevel = 1200;
  bool ledOn = false;
};
