# D03–D07 implementation and handoff

## D03 — Hardware and firmware integration

- Pin assignment follows `docs/PINOUT.md`: MPU6050 SDA/SCL GPIO21/22, SOS GPIO13, buzzer driver GPIO14.
- Copy `firmware/include/WifiSecrets.example.h` to the gitignored `firmware/include/WifiSecrets.h` and enter local Wi-Fi/MQTT settings.
- PlatformIO now includes the Adafruit MPU6050 driver. Firmware calibrates the upright acceleration vector for two seconds at boot; hold the assembled cane upright and still during this period.
- The safety state machine latches a short impact/free-fall sample, confirms a stable tilt, starts a 10-second cancel window, then emits `CANE_FALL_ALERT`. Thresholds in `firmware/src/SafetyModule.cpp` are provisional; use `docs/REPORT_TEMPLATE_D03_D07.md` to record measured adjustments.
- Test on USB first. Verify all grounds, 3.3V I2C logic, and a transistor/driver for the buzzer if its current exceeds the GPIO rating. Do not connect a 5V MPU signal or buzzer load directly to an ESP32 GPIO.

## D04 — Telegram backend

1. Create a bot with Telegram's BotFather, start a private chat with it, and obtain the chat ID for that chat.
2. Set `TELEGRAM_BOT_TOKEN` and `TELEGRAM_CHAT_ID` in the backend process environment. Never put real credentials in `.env.example`, firmware, or Git.
3. The backend sends only committed `SOS` and `CANE_FALL_ALERT` incidents. It sends a Maps link only when `gps_valid=true` and both coordinates pass backend validation. `CANCEL` and `CANE_FALL_SUSPECTED` are stored but do not notify.
4. Replayed `event_id` values are ignored by `MessageService`; this prevents duplicate notifications when the backend receives a replay after the first save.
5. Backend logs include `[TELEGRAM] sent ... end_to_end_ms=...`; record these values during D06. Without credentials, notifications are skipped while event storage continues.
6. Telegram HTTP work runs on a bounded background executor so it does not hold the MQTT event consumer during normal traffic. The queue is in memory; a backend process crash can lose queued notifications.

## D05 — Replayable tests

- Install Python dependencies with `py -m pip install -r tests/requirements.txt`.
- `tests/safety-events.jsonl` contains synthetic valid contract events. `tests/SAFETY_SCENARIOS.md` describes expected behavior and invalid-input cases.
- Backend JUnit tests cover parsing, topic/device matching, idempotency, GPS validation, and Telegram message location formatting.
- The platform-independent firmware state machine has a native harness: `g++ -std=c++17 -Ifirmware/include tests/SafetyModuleTest.cpp firmware/src/SafetyModule.cpp -o safety-module-test.exe`, then run `./safety-module-test.exe` (PowerShell: `./safety-module-test.exe`).

## D06 — Integration measurement

Start Mosquitto/PostgreSQL and the backend, then run:

```powershell
py tests/d06_integration.py --repeat 5
```

This default run exercises non-notifying event cases. To also send synthetic SOS/fall alerts through a configured Telegram bot, use `--include-notification-cases` only with a test bot/chat. The runner saves `tests/d06-results.json` and measures MQTT publish to incident visibility through the API. Telegram round-trip values are measured by the backend logs and must be transcribed into the report; do not treat API persistence time as Telegram delivery time.

For offline validation, disconnect Wi-Fi after the firmware is running, trigger SOS, and verify the local buzzer still sounds. Record that run manually; the MQTT replay tool cannot prove physical offline behavior.

## D07 — Report and demo

Use `docs/REPORT_TEMPLATE_D03_D07.md` and follow `docs/DEMO_SCRIPT_D07.md`. Attach real Serial/MQTT/backend/Telegram evidence. Leave unrun rows marked pending and do not claim the prototype detects a person's fall.

## Current implementation limits

- Firmware telemetry still contains placeholder distance/water/light values owned by other modules.
- Firmware events use ArduinoMqttClient at QoS 1, and `endMessage()` waits for PUBACK. Validate this on the team's actual broker before claiming delivery guarantees in the report.
- Firmware MQTT connect/PUBACK waits are capped at one second so the safety loop can resume; measure local button/buzzer behavior while the broker is unavailable.
- Firmware does not persist events while offline. Local SOS/buzzer behavior remains available, but queued delivery after reconnect needs a durable outbox.
- Hardware, Telegram credentials, and physical results must be supplied by the team; this code does not fabricate those results.
