#include <Wire.h>
#include <math.h>

// Smart Cane D01/D02 Wokwi demo: MPU6050 + SOS button + buzzer.
// Button is active LOW (GPIO13 to GND). MPU6050 uses I2C GPIO21/22.
constexpr uint8_t MPU_ADDR = 0x68;
constexpr uint8_t BUTTON_PIN = 13;
constexpr uint8_t BUZZER_PIN = 14;
constexpr uint32_t DEBOUNCE_MS = 35;
constexpr uint32_t SOS_HOLD_MS = 3000;
constexpr uint32_t COUNTDOWN_MS = 10000;
constexpr uint32_t BEEP_MS = 250;
constexpr float FREEFALL_MAX_G = 0.55f;
constexpr float IMPACT_MIN_G = 1.80f;
constexpr float TILT_LIMIT_DEG = 60.0f;
constexpr uint32_t FALL_CONFIRM_MS = 600;

enum class FallState : uint8_t { NORMAL, COUNTDOWN, ALERT };
struct SafetyEvent { const char* type; uint32_t atMs; };

FallState fallState = FallState::NORMAL;
bool rawButton = false, stableButton = false, sosSent = false;
bool fallCandidate = false, buzzerOn = false;
uint32_t buttonChangedAt = 0, buttonPressedAt = 0;
uint32_t fallCandidateAt = 0, countdownStartedAt = 0, beepUntil = 0;

bool readMpu(float& accelG, float& tiltDeg) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // ACCEL_XOUT_H
  if (Wire.endTransmission(false) != 0 || Wire.requestFrom(MPU_ADDR, (uint8_t)6) != 6) return false;
  int16_t ax = (int16_t)((Wire.read() << 8) | Wire.read());
  int16_t ay = (int16_t)((Wire.read() << 8) | Wire.read());
  int16_t az = (int16_t)((Wire.read() << 8) | Wire.read());
  float x = ax / 16384.0f, y = ay / 16384.0f, z = az / 16384.0f;
  accelG = sqrtf(x*x + y*y + z*z);
  tiltDeg = atan2f(sqrtf(x*x + y*y), fabsf(z)) * 57.29578f;
  return true;
}

void event(const char* type, uint32_t now) {
  Serial.printf("[EVENT] %s at %lu ms\n", type, (unsigned long)now);
}

void updateSafety(bool pressed, float accelG, float tiltDeg, uint32_t now) {
  if (pressed != rawButton) {
    rawButton = pressed;
    buttonChangedAt = now;
  }

  if (rawButton != stableButton && now - buttonChangedAt >= DEBOUNCE_MS) {
    stableButton = rawButton;
    if (stableButton) {
      buttonPressedAt = now;
      sosSent = false;
      if (fallState == FallState::COUNTDOWN && now - countdownStartedAt < COUNTDOWN_MS) {
        fallState = FallState::NORMAL;
        fallCandidate = false;
        sosSent = true; // the cancel press must not also become an SOS hold
        beepUntil = now + BEEP_MS;
        event("CANCEL", now);
      }
    }
  }

  if (stableButton && !sosSent && now - buttonPressedAt >= SOS_HOLD_MS) {
    sosSent = true;
    beepUntil = now + BEEP_MS;
    event("SOS", now);
  }

  if (fallState == FallState::NORMAL) {
    bool impact = accelG <= FREEFALL_MAX_G || accelG >= IMPACT_MIN_G;
    bool tilted = fabsf(tiltDeg) >= TILT_LIMIT_DEG;
    // Latch a brief impact while the cane stays tilted for the confirmation time.
    if (impact && tilted && !fallCandidate) {
      fallCandidate = true;
      fallCandidateAt = now;
    }
    if (fallCandidate && !tilted) fallCandidate = false;
    else if (fallCandidate && now - fallCandidateAt >= FALL_CONFIRM_MS) {
      fallState = FallState::COUNTDOWN;
      countdownStartedAt = now;
      fallCandidate = false;
      beepUntil = now + BEEP_MS;
      event("CANE_FALL_SUSPECTED", now);
      Serial.println("[INFO] Countdown 10s: press button once to cancel.");
    }
  } else if (fallState == FallState::COUNTDOWN && now - countdownStartedAt >= COUNTDOWN_MS) {
    fallState = FallState::ALERT;
    beepUntil = now + BEEP_MS;
    event("CANE_FALL_ALERT", now);
  }

  buzzerOn = (stableButton && !sosSent) || now < beepUntil;
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  Wire.begin(21, 22);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); Wire.write(0); // wake MPU6050
  bool mpuReady = Wire.endTransmission() == 0;
  rawButton = stableButton = digitalRead(BUTTON_PIN) == LOW;
  buttonPressedAt = buttonChangedAt = millis();
  Serial.printf("Smart Cane D01/D02 | MPU6050: %s\n", mpuReady ? "OK" : "NOT FOUND");
  Serial.println("Hold SOS button 3s. Tilt + impact simulates suspected cane fall.");
  Serial.println("This demo detects cane-fall suspicion, not a person's fall.");
}

void loop() {
  uint32_t now = millis();
  float accelG = 1.0f, tiltDeg = 0.0f;
  bool sensorOk = readMpu(accelG, tiltDeg);
  updateSafety(digitalRead(BUTTON_PIN) == LOW,
               sensorOk ? accelG : 1.0f,
               sensorOk ? tiltDeg : 0.0f, now);
  if (buzzerOn) tone(BUZZER_PIN, 2000);
  else noTone(BUZZER_PIN);
  delay(5);
}
