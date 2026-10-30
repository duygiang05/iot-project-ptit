# Smart Cane backend

Spring Boot 3.3, Java 21, MQTT event/telemetry consumer, PostgreSQL, REST read API, and optional Telegram alerts.

## Local run

Start Mosquitto and PostgreSQL from the project root:

```powershell
docker compose --env-file .env -f infra/compose.yaml up -d
```

Set database credentials in the backend process environment, then run:

```powershell
$env:DB_PASSWORD = '<local PostgreSQL password>'
$env:TELEGRAM_BOT_TOKEN = '<bot token>'
$env:TELEGRAM_CHAT_ID = '<test chat id>'
mvn test
mvn spring-boot:run
```

Telegram variables are optional. If absent, events are still saved and Telegram is skipped. Configure them only in a local environment; never commit real credentials. Use a test bot/chat while running notification integration cases.

The backend notifies for committed SOS and `CANE_FALL_ALERT` incidents, includes a Maps URL only for validated GPS coordinates, and ignores duplicate `event_id` values. `CANCEL` and `CANE_FALL_SUSPECTED` are stored without Telegram notifications.

Read API: `/api/devices/{deviceId}/incidents?limit=20` and `/api/devices/{deviceId}/telemetry/latest`.
