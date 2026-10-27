// Smart Cane D01/D02 Wokwi demo with deterministic fake sensor input.
// Serial commands: F = fake fall, C = cancel countdown, S = SOS, R = reset.
// Physical button: hold 3s for SOS; press once during countdown to cancel.
constexpr uint8_t BUTTON_PIN = 13;
constexpr uint8_t BUZZER_PIN = 25;
constexpr uint32_t DEBOUNCE_MS = 35;
constexpr uint32_t SOS_HOLD_MS = 3000;
constexpr uint32_t FALL_CONFIRM_MS = 600;
constexpr uint32_t COUNTDOWN_MS = 10000;
constexpr uint32_t COUNTDOWN_TICK_MS = 1000;

enum class CaneState : uint8_t { NORMAL, SUSPECTED, COUNTDOWN, ALERT };
CaneState state = CaneState::NORMAL;

bool rawButtonDown = false;
bool stableButtonDown = false;
bool sosSentForPress = false;
bool cancelPress = false;
uint32_t rawChangedAt = 0;
uint32_t buttonPressedAt = 0;
uint32_t suspectedAt = 0;
uint32_t countdownAt = 0;
uint32_t lastCountdownTick = 0;
uint32_t eventCounter = 0;

const char* stateName(CaneState value) {
  switch (value) {
    case CaneState::NORMAL: return "NORMAL";
    case CaneState::SUSPECTED: return "SUSPECTED";
    case CaneState::COUNTDOWN: return "COUNTDOWN";
    case CaneState::ALERT: return "ALERT";
  }
  return "UNKNOWN";
}

void publishEvent(const char* type) {
  // JSON-shaped Serial output for demo/replay. This sketch does not publish MQTT.
  Serial.printf("[EVENT] {\"event_id\":\"wokwi-%lu\",\"device_id\":\"cane-demo\",\"type\":\"%s\",\"gps_valid\":false,\"latitude\":null,\"longitude\":null}\n",
                (unsigned long)++eventCounter, type);
}

void changeState(CaneState next) {
  if (state == next) return;
  state = next;
  Serial.printf("[STATE] %s\n", stateName(state));
}

void startFakeFall(uint32_t now) {
  if (state != CaneState::NORMAL) {
    Serial.println("[INFO] F ignored: device is not in NORMAL state. Send R to reset.");
    return;
  }
  // Fake values simulate a short impact and a cane tilted on the ground.
  const float fakeAccelG = 2.0f;
  const float fakeTiltDeg = 75.0f;
  Serial.printf("[FAKE SENSOR] acceleration=%.1fg tilt=%.0fdeg\n", fakeAccelG, fakeTiltDeg);
  suspectedAt = now;
  changeState(CaneState::SUSPECTED);
  publishEvent("CANE_FALL_SUSPECTED");
}

void sendSos(uint32_t now) {
  if (state != CaneState::NORMAL) {
    Serial.println("[INFO] SOS ignored while fall flow is active.");
    return;
  }
  publishEvent("SOS");
  tone(BUZZER_PIN, 2000, 500);
  (void)now;
}

void cancelCountdown() {
  if (state != CaneState::COUNTDOWN) {
    Serial.println("[INFO] Nothing to cancel; countdown is not active.");
    return;
  }
  noTone(BUZZER_PIN);
  changeState(CaneState::NORMAL);
  publishEvent("CANCEL");
  Serial.println("[RESULT] Fall warning cancelled.");
}

void resetDemo() {
  noTone(BUZZER_PIN);
  changeState(CaneState::NORMAL);
  Serial.println("[INFO] Demo reset. Send F to test a fake fall.");
}

void handleSerialCommand(uint32_t now) {
  while (Serial.available() > 0) {
    char command = Serial.read();
    if (command >= 'a' && command <= 'z') command -= ('a' - 'A');
    switch (command) {
      case 'F': startFakeFall(now); break;
      case 'C': cancelCountdown(); break;
      case 'S': sendSos(now); break;
      case 'R': resetDemo(); break;
      case '\r':
      case '\n':
      case ' ': break;
      default: Serial.println("[HELP] Commands: F fake fall, C cancel, S SOS, R reset."); break;
    }
  }
}

void handleButton(uint32_t now) {
  bool pressed = digitalRead(BUTTON_PIN) == LOW;
  if (pressed != rawButtonDown) {
    rawButtonDown = pressed;
    rawChangedAt = now;
  }
  if (rawButtonDown != stableButtonDown && now - rawChangedAt >= DEBOUNCE_MS) {
    stableButtonDown = rawButtonDown;
    if (stableButtonDown) {
      buttonPressedAt = now;
      sosSentForPress = false;
      cancelPress = state == CaneState::COUNTDOWN;
      if (cancelPress) cancelCountdown();
    } else {
      cancelPress = false;
    }
  }

  if (stableButtonDown && state == CaneState::NORMAL && !sosSentForPress &&
      !cancelPress && now - buttonPressedAt >= SOS_HOLD_MS) {
    sendSos(now);
    sosSentForPress = true;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  rawButtonDown = stableButtonDown = digitalRead(BUTTON_PIN) == LOW;
  rawChangedAt = buttonPressedAt = millis();
  Serial.println("Smart Cane Wokwi fake-data demo ready.");
  Serial.println("Send F for fall warning, C to cancel, S for SOS, R to reset.");
  Serial.println("Or hold the physical button 3s for SOS; press it during countdown to cancel.");
  Serial.println("Fall output means suspected cane fall only, not a confirmed person fall.");
  Serial.println("[STATE] NORMAL");
}

void loop() {
  const uint32_t now = millis();
  handleSerialCommand(now);
  handleButton(now);

  if (state == CaneState::SUSPECTED && now - suspectedAt >= FALL_CONFIRM_MS) {
    countdownAt = now;
    lastCountdownTick = now;
    changeState(CaneState::COUNTDOWN);
    Serial.println("[INFO] 10-second countdown started. Send C or press button to cancel.");
  }

  if (state == CaneState::COUNTDOWN) {
    uint32_t elapsed = now - countdownAt;
    if (elapsed >= COUNTDOWN_MS) {
      changeState(CaneState::ALERT);
      publishEvent("CANE_FALL_ALERT");
      tone(BUZZER_PIN, 2000);
      Serial.println("[ALERT] FALL WARNING: suspected cane fall was not cancelled.");
    } else if (now - lastCountdownTick >= COUNTDOWN_TICK_MS) {
      lastCountdownTick = now;
      uint32_t secondsLeft = (COUNTDOWN_MS - elapsed + 999) / 1000;
      Serial.printf("[COUNTDOWN] %lu seconds left\n", (unsigned long)secondsLeft);
    }
  }

  delay(5);
}
