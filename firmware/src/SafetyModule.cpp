#include "SafetyModule.h"
#include <math.h>

namespace {
// Deliberately conservative demo thresholds; validate on the cane and tune.
constexpr float FALL_FREEFALL_MAX_G = 0.55f;
constexpr float FALL_IMPACT_MIN_G = 1.80f;
constexpr float FALL_TILT_DEGREES = 60.0f;
constexpr uint32_t FALL_CONFIRM_MS = 600;
}

void SafetyModule::begin(bool buttonPressed, uint32_t nowMs) {
  initialized_ = true;
  rawPressed_ = stablePressed_ = buttonPressed;
  rawChangedAt_ = nowMs;
  pressedAt_ = nowMs;
}

void SafetyModule::emit(SafetyEventType type, uint32_t nowMs) {
  // One-slot queue: don't overwrite an event that the publisher hasn't read.
  if (eventPending_) return;
  queuedEvent_ = {type, nowMs};
  eventPending_ = true;
}

bool SafetyModule::popEvent(SafetyEvent& event) {
  if (!eventPending_) return false;
  event = queuedEvent_;
  eventPending_ = false;
  return true;
}

void SafetyModule::setButton(bool pressed, uint32_t nowMs) {
  if (pressed == rawPressed_) return;
  rawPressed_ = pressed;
  rawChangedAt_ = nowMs;
}

void SafetyModule::update(bool buttonPressed, float accelerationG,
                          float tiltDegrees, uint32_t nowMs) {
  if (!initialized_) begin(buttonPressed, nowMs);
  setButton(buttonPressed, nowMs);
  if (rawPressed_ != stablePressed_ && nowMs - rawChangedAt_ >= BUTTON_DEBOUNCE_MS) {
    stablePressed_ = rawPressed_;
    if (stablePressed_) {
      pressedAt_ = nowMs;
      holdHandled_ = false;
      if (fallState_ == FallState::SUSPECTED &&
          nowMs - suspectedAt_ < CANCEL_WINDOW_MS) {
        fallState_ = FallState::NORMAL;
        fallCandidate_ = false;
        holdHandled_ = true;
        beepUntil_ = nowMs + ALERT_BEEP_MS;
        emit(SafetyEventType::CANCEL, nowMs);
      }
    }
  }

  if (stablePressed_ && !holdHandled_ && nowMs - pressedAt_ >= SOS_HOLD_MS) {
    holdHandled_ = true;
    beepUntil_ = nowMs + ALERT_BEEP_MS;
    emit(SafetyEventType::SOS, nowMs);
  }

  if (fallState_ == FallState::NORMAL) {
    const bool impact = accelerationG <= FALL_FREEFALL_MAX_G ||
                        accelerationG >= FALL_IMPACT_MIN_G;
    const bool tilted = fabsf(tiltDegrees) >= FALL_TILT_DEGREES;
    if (impact && tilted && !fallCandidate_) {
      // Latch the short impact/free-fall sample while the cane remains tilted.
      fallCandidate_ = true;
      suspectedAt_ = nowMs;
    }
    if (fallCandidate_ && !tilted) {
      fallCandidate_ = false;
    } else if (fallCandidate_ && nowMs - suspectedAt_ >= FALL_CONFIRM_MS) {
      fallState_ = FallState::SUSPECTED;
      suspectedAt_ = nowMs;
      beepUntil_ = nowMs + ALERT_BEEP_MS;
      emit(SafetyEventType::CANE_FALL_SUSPECTED, nowMs);
    }
  } else if (fallState_ == FallState::SUSPECTED) {
    if (nowMs - suspectedAt_ >= CANCEL_WINDOW_MS) {
      fallState_ = FallState::ALERT;
      beepUntil_ = nowMs + ALERT_BEEP_MS;
      emit(SafetyEventType::CANE_FALL_ALERT, nowMs);
    }
  }

  // SOS beeps while pressed; a fall warning chirps briefly and never blocks.
  buzzerOn_ = (stablePressed_ && !holdHandled_) || nowMs < beepUntil_;
}
