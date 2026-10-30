#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

// Smart Cane D01/D02: real MPU6050 readings, SOS button, countdown and buzzer.
// Hold the cane upright and still during the startup calibration.
constexpr uint8_t BUTTON_PIN = 13;
constexpr uint8_t BUZZER_PIN = 25;
constexpr uint8_t MPU_SDA_PIN = 21;
constexpr uint8_t MPU_SCL_PIN = 22;
constexpr uint32_t DEBOUNCE_MS = 35;
constexpr uint32_t SOS_HOLD_MS = 3000;
constexpr uint32_t FALL_CONFIRM_MS = 600;
constexpr uint32_t COUNTDOWN_MS = 10000;
constexpr uint32_t SAMPLE_MS = 50;
constexpr uint32_t IMPACT_MEMORY_MS = 3000;

// Prototype thresholds. Tune these with recorded data from the assembled cane.
constexpr float FREEFALL_MAX_G = 0.55f;
constexpr float IMPACT_MIN_G = 1.80f;
constexpr float STABLE_MIN_G = 0.75f;
constexpr float STABLE_MAX_G = 1.25f;
constexpr float TILT_TRIGGER_DEG = 60.0f;
constexpr float TILT_RESET_DEG = 45.0f;

enum class CaneState : uint8_t { NORMAL, SUSPECTED, COUNTDOWN, ALERT };
Adafruit_MPU6050 mpu;
CaneState state = CaneState::NORMAL;

float uprightX = 0.0f, uprightY = 0.0f, uprightZ = 1.0f;
bool sensorReady = false;
bool rawButtonDown = false, stableButtonDown = false;
bool sosSentForPress = false, impactSeen = false;
uint32_t rawChangedAt = 0, buttonPressedAt = 0, suspectedAt = 0;
uint32_t countdownAt = 0, lastSampleAt = 0, lastImpactAt = 0;
uint32_t lastCountdownTick = 0, lastSensorLogAt = 0, eventCounter = 0;

const char* stateName(CaneState value) {
  switch (value) {
    case CaneState::NORMAL: return "NORMAL";
    case CaneState::SUSPECTED: return "SUSPECTED";
    case CaneState::COUNTDOWN: return "COUNTDOWN";
    case CaneState::ALERT: return "ALERT";
  }
  return "UNKNOWN";
}

void changeState(CaneState next) {
  if (state == next) return;
  state = next;
  Serial.printf("[STATE] %s\n", stateName(state));
}

void publishEvent(const char* type) {
  // JSON follows the shared event fields. Wokwi/Serial demo only; no MQTT here.
  Serial.printf("[EVENT] {\"event_id\":\"cane-01-%lu-%lu\",\"device_id\":\"cane-01\",\"type\":\"%s\",\"latitude\":null,\"longitude\":null,\"gps_valid\":false}\n",
                (unsigned long)millis(), (unsigned long)++eventCounter, type);
}

bool readAcceleration(float& x, float& y, float& z, float& magnitudeG) {
  if (!sensorReady) return false;
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);
  x = accel.acceleration.x;
  y = accel.acceleration.y;
  z = accel.acceleration.z;
  magnitudeG = sqrtf(x*x + y*y + z*z) / 9.80665f;
  return isfinite(magnitudeG);
}

bool calibrateUpright() {
  Serial.println("[CAL] Hold cane upright and still for 2 seconds...");
  float sx = 0, sy = 0, sz = 0;
  uint8_t samples = 0;
  const uint32_t start = millis();
  while (millis() - start < 2000) {
    float x, y, z, magnitudeG;
    if (readAcceleration(x, y, z, magnitudeG) &&
        magnitudeG >= STABLE_MIN_G && magnitudeG <= STABLE_MAX_G) {
      sx += x / (magnitudeG * 9.80665f);
      sy += y / (magnitudeG * 9.80665f);
      sz += z / (magnitudeG * 9.80665f);
      ++samples;
    }
    delay(50);
  }
  if (samples < 10) return false;
  const float length = sqrtf(sx*sx + sy*sy + sz*sz);
  if (length < 0.1f) return false;
  uprightX = sx / length;
  uprightY = sy / length;
  uprightZ = sz / length;
  return true;
}

void sendSos() {
  if (state != CaneState::NORMAL) {
    Serial.println("[INFO] SOS button is unavailable while fall flow is active.");
    return;
  }
  publishEvent("SOS");
  tone(BUZZER_PIN, 2000, 500);
}

void cancelCountdown() {
  if (state != CaneState::COUNTDOWN) return;
  noTone(BUZZER_PIN);
  changeState(CaneState::NORMAL);
  impactSeen = false;
  publishEvent("CANCEL");
  tone(BUZZER_PIN, 1400, 150);
  Serial.println("[RESULT] Countdown cancelled by user.");
}

void handleButton(uint32_t now) {
  const bool pressed = digitalRead(BUTTON_PIN) == LOW;
  if (pressed != rawButtonDown) {
    rawButtonDown = pressed;
    rawChangedAt = now;
  }
  if (rawButtonDown != stableButtonDown && now - rawChangedAt >= DEBOUNCE_MS) {
    stableButtonDown = rawButtonDown;
    if (stableButtonDown) {
      buttonPressedAt = now;
      sosSentForPress = false;
      if (state == CaneState::COUNTDOWN) {
        cancelCountdown();
        sosSentForPress = true;
      } else if (state == CaneState::ALERT) {
        noTone(BUZZER_PIN);
        Serial.println("[INFO] Buzzer muted; alert state remains active.");
      }
    }
  }

  if (stableButtonDown && state == CaneState::NORMAL && !sosSentForPress &&
      now - buttonPressedAt >= SOS_HOLD_MS) {
    sendSos();
    sosSentForPress = true;
  }
}

void sampleFallSensor(uint32_t now) {
  if (!sensorReady || now - lastSampleAt < SAMPLE_MS) return;
  lastSampleAt = now;

  float x, y, z, magnitudeG;
  if (!readAcceleration(x, y, z, magnitudeG)) {
    Serial.println("[SENSOR] MPU6050 read failed.");
    return;
  }

  // Brief impact/free-fall is latched; tilt is judged on near-gravity samples,
  // when accelerometer orientation is a useful estimate of cane inclination.
  if (magnitudeG <= FREEFALL_MAX_G || magnitudeG >= IMPACT_MIN_G) {
    impactSeen = true;
    lastImpactAt = now;
  }
  if (impactSeen && now - lastImpactAt > IMPACT_MEMORY_MS) impactSeen = false;

  float tiltDeg = 0.0f;
  const bool gravityStable = magnitudeG >= STABLE_MIN_G && magnitudeG <= STABLE_MAX_G;
  if (gravityStable) {
    const float length = magnitudeG * 9.80665f;
    float dot = (x/length)*uprightX + (y/length)*uprightY + (z/length)*uprightZ;
    dot = constrain(dot, -1.0f, 1.0f);
    tiltDeg = acosf(dot) * 57.29578f;
  }

  if (now - lastSensorLogAt >= 250) {
    lastSensorLogAt = now;
    Serial.printf("[SENSOR] %.2fg | tilt %.1f deg | impact %s\n",
                  magnitudeG, tiltDeg, impactSeen ? "latched" : "no");
  }

  if (state == CaneState::NORMAL && impactSeen && gravityStable &&
      tiltDeg >= TILT_TRIGGER_DEG) {
    suspectedAt = now;
    changeState(CaneState::SUSPECTED);
  } else if (state == CaneState::SUSPECTED) {
    if (!impactSeen || !gravityStable || tiltDeg < TILT_RESET_DEG) {
      changeState(CaneState::NORMAL);
      Serial.println("[INFO] Suspected movement cleared before confirmation.");
    } else if (now - suspectedAt >= FALL_CONFIRM_MS) {
      changeState(CaneState::COUNTDOWN);
      countdownAt = lastCountdownTick = now;
      publishEvent("CANE_FALL_SUSPECTED");
      Serial.println("[INFO] 10-second countdown. Press the button to cancel.");
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  Wire.begin(MPU_SDA_PIN, MPU_SCL_PIN);
  sensorReady = mpu.begin(0x68, &Wire);
  rawButtonDown = stableButtonDown = digitalRead(BUTTON_PIN) == LOW;
  rawChangedAt = buttonPressedAt = millis();

  if (!sensorReady) {
    Serial.println("[ERROR] MPU6050 not found. Check D21/SDA, D22/SCL, 3V3 and GND.");
    Serial.println("[INFO] Fall detection is disabled; SOS button still works.");
  } else {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    if (calibrateUpright()) Serial.println("[CAL] Upright reference saved.");
    else Serial.println("[CAL] Not enough stable samples; using default vertical reference.");
  }

  Serial.println("Smart Cane real-sensor sketch ready.");
  Serial.println("Hold the button 3s for SOS; press once during countdown to cancel.");
  Serial.println("Fall detection means suspected cane fall only, not a confirmed person fall.");
  Serial.println("[STATE] NORMAL");
}

void loop() {
  const uint32_t now = millis();
  handleButton(now);
  sampleFallSensor(now);

  if (state == CaneState::COUNTDOWN) {
    const uint32_t elapsed = now - countdownAt;
    if (elapsed >= COUNTDOWN_MS) {
      changeState(CaneState::ALERT);
      publishEvent("CANE_FALL_ALERT");
      tone(BUZZER_PIN, 2000);
      Serial.println("[ALERT] Suspected cane fall was not cancelled.");
    } else if (now - lastCountdownTick >= 1000) {
      lastCountdownTick = now;
      const uint32_t secondsLeft = (COUNTDOWN_MS - elapsed + 999) / 1000;
      Serial.printf("[COUNTDOWN] %lu seconds left\n", (unsigned long)secondsLeft);
    }
  }

  delay(5);
}
