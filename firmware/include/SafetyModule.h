#pragma once

#include <stdint.h>

// Sensor-independent safety state machine. Caller supplies normalized MPU6050
// samples so thresholds can be calibrated without changing event behavior.
enum class SafetyEventType : uint8_t {
  NONE, SOS, CANE_FALL_SUSPECTED, CANE_FALL_ALERT, CANCEL
};

struct SafetyEvent {
  SafetyEventType type = SafetyEventType::NONE;
  uint32_t occurredAtMs = 0;
};

class SafetyModule {
 public:
  static constexpr uint32_t BUTTON_DEBOUNCE_MS = 35;
  static constexpr uint32_t SOS_HOLD_MS = 3000;
  static constexpr uint32_t CANCEL_WINDOW_MS = 10000;
  static constexpr uint32_t ALERT_BEEP_MS = 250;

  void begin(bool buttonPressed, uint32_t nowMs);
  void update(bool buttonPressed, float accelerationG, float tiltDegrees,
              uint32_t nowMs);
  bool buzzerOn() const { return buzzerOn_; }
  bool popEvent(SafetyEvent& event);

 private:
  enum class FallState : uint8_t { NORMAL, SUSPECTED, ALERT };
  void emit(SafetyEventType type, uint32_t nowMs);
  void setButton(bool pressed, uint32_t nowMs);

  bool initialized_ = false;
  bool rawPressed_ = false;
  bool stablePressed_ = false;
  bool holdHandled_ = false;
  bool buzzerOn_ = false;
  bool eventPending_ = false;
  bool fallCandidate_ = false;
  uint32_t rawChangedAt_ = 0;
  uint32_t pressedAt_ = 0;
  uint32_t suspectedAt_ = 0;
  uint32_t beepUntil_ = 0;
  FallState fallState_ = FallState::NORMAL;
  SafetyEvent queuedEvent_;
};
