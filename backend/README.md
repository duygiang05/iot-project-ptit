# Smart Cane IoT Backend (G03 + G04)

Java 21, Spring Boot 3.3.13, Spring Integration MQTT, PostgreSQL 16.

## Install
Extract this ZIP into `D:\IOT-PROJECT` so `backend/pom.xml` exists. Do **not** delete sibling folders.

## Prerequisites
- Java 21 and Maven 3.9+
- Existing Docker services: Mosquitto 127.0.0.1:1883; PostgreSQL 127.0.0.1:5433
- DB `smartcane`, user `smartcane`, password configured in the existing Docker `.env`.

## Run (PowerShell)
```powershell
cd D:\IOT-PROJECT
docker compose --env-file .env -f infra/compose.yaml ps
$env:DB_PASSWORD = '<your local PostgreSQL password>'
cd backend
mvn clean test
mvn spring-boot:run
```

API: http://localhost:8081/api/devices/cane-01/telemetry/latest and http://localhost:8081/api/devices/cane-01/incidents.

MQTT topics: `smartcane/+/telemetry`, `smartcane/+/events`. Keep Bookstore 8080 unaffected.

**Note:** only backend, no Wokwi/firmware/frontend included. G03/G04 require live tests. SQL schema uses Hibernate `ddl-auto: update` for local dev only. DB credential is an environment variable, never commit the real secret.
