# Smart Cane IoT — PTIT

Prototype project for an IoT cane with SOS and suspected cane-fall alerts. It is not a certified medical or emergency device and does not confirm that a person has fallen.

## Quick start

1. Copy `.env.example` to `.env`, set a local PostgreSQL password, then run `docker compose --env-file .env -f infra/compose.yaml up -d`.
2. Run the Spring backend from `backend/` with Java 21 and Maven: `mvn test`, then `mvn spring-boot:run`.
3. To enable Telegram, set `TELEGRAM_BOT_TOKEN` and `TELEGRAM_CHAT_ID` in the backend process environment. Keep real credentials out of Git.
4. For ESP32 firmware, copy `firmware/include/WifiSecrets.example.h` to the gitignored `WifiSecrets.h`, enter local Wi-Fi/MQTT values, then build with `pio run -d firmware`.
5. Open `simulation/duy/` for the Wokwi safety simulation. Open `docs/D03-D07_GUIDE.md` for wiring, integration tests, report and demo instructions.

## Project layout

- `firmware/`: integrated ESP32 MPU6050 safety logic, SOS, buzzer, Wi-Fi and MQTT.
- `backend/`: Spring Boot MQTT consumer, PostgreSQL records, read API and optional Telegram alerts.
- `simulation/`: member Wokwi projects.
- `tests/`: JSON fixtures, native safety-state test and repeated MQTT-to-API integration runner.
- `docs/`: pinout, MQTT contract, D03–D07 guide, report template and demo run sheet.

## Current limits

- MPU thresholds are provisional and must be tuned from measured samples on the assembled cane.
- Firmware sends `SOS`, `CANE_FALL_SUSPECTED`, `CANE_FALL_ALERT`, and `CANCEL` events at QoS 1. Offline event persistence is not implemented; local SOS/buzzer behavior still runs without Wi-Fi.
- Distance, water, light, and GPS telemetry values remain placeholders for their module owners.
- Record actual Wokwi, hardware, Telegram, and latency evidence in the D03–D07 report; do not mark unrun checks as passed.
