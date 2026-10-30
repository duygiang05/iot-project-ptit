# Firmware ESP32 — PlatformIO / Arduino

The integrated firmware reads the MPU6050, debounces the SOS button, sounds the local buzzer, prints safety events, and publishes telemetry/events when MQTT is connected. The safety input and local buzzer continue to run while Wi-Fi is offline.

## D03 setup

1. Copy `include/WifiSecrets.example.h` to `include/WifiSecrets.h` and fill local Wi-Fi and MQTT values. The real header is gitignored.
2. Hold the cane upright and still during the 2-second MPU6050 calibration at boot.
3. Build with `pio run -d firmware`; monitor with `pio device monitor -d firmware -b 115200`.
4. Use the pinout in `docs/PINOUT.md`: MPU6050 SDA/SCL GPIO21/22, SOS GPIO13, buzzer driver GPIO14.

The Adafruit MPU6050 driver is declared in `platformio.ini`. Thresholds in `src/SafetyModule.cpp` are provisional and must be tuned from actual readings; this is a cane-fall suspicion demo, not a person-fall detector.

## Known implementation limits

- Distance, water, light, and GPS telemetry fields are still placeholders owned by other module tasks.
- Events publish at QoS 1 through ArduinoMqttClient and wait for PUBACK. Verify acknowledgments on the team's broker.
- Events are not queued durably while MQTT is offline. Local SOS/buzzer behavior remains available, but the backend may not receive an event triggered offline.
- Hardware measurements and the real Telegram round-trip must be recorded by the team; see `docs/D03-D07_GUIDE.md`.
