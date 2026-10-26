# G06 - ESP32 that -> Wi-Fi LAN -> Mosquitto Docker -> Spring Boot -> PostgreSQL

## 0. Scope
Real ESP32 DevKit, connected via USB. Do not connect sensors/motor/battery yet. Sends synthetic telemetry to validate the real Wi-Fi/MQTT path. `readTelemetry()` is the interface point for Tung/Duy/Dat during G09. The ESP32 does **not** use public HiveMQ or the Python relay.

## 1. Software and hardware
- ESP32 DevKit CH340 (30 pin), known data USB cable, Windows computer, 2.4GHz Wi-Fi router/hotspot on same LAN as Windows PC.
- VS Code with PlatformIO extension; use `D:\IOT-PROJECT\firmware` as the project folder. Verify USB COM port in Device Manager; install CH340 driver if necessary.
- Docker Mosquitto and PostgreSQL up and Spring Boot 8081 running.

## 2. Prepare networking (crucial)
Current `infra/compose.yaml` originally binds MQTT as `127.0.0.1:1883:1883`. Real ESP32 on LAN cannot reach it.
In `D:\IOT-PROJECT\infra\compose.yaml`, change ONLY Mosquitto ports line to:

```yaml
    ports:
      - "1883:1883"  # DEV-only on trusted LAN; do NOT forward this port on the router
```

Keep PostgreSQL `127.0.0.1:5433:5432` unchanged.
Run from project root:

```powershell
docker compose --env-file .env -f infra/compose.yaml up -d --force-recreate mosquitto
docker compose --env-file .env -f infra/compose.yaml ps
```

Find Windows Wi-Fi IPv4 (not 127.0.0.1, not Docker, not CloudflareWARP) with `ipconfig` / `Get-NetIPAddress -AddressFamily IPv4`. Example: `192.168.1.100`.
Test Windows -> mapped IP: `Test-NetConnection 192.168.1.100 -Port 1883` (replace address).
Allow TCP 1883 on Windows Private network profile ONLY and remote LocalSubnet using Windows Firewall if blocked (administrator privileges):

```powershell
New-NetFirewallRule -DisplayName 'SmartCane MQTT LAN DEV' -Direction Inbound -Action Allow -Protocol TCP -LocalPort 1883 -Profile Private -RemoteAddress LocalSubnet
```

Broker currently uses `allow_anonymous true` (local demo only). Use trusted/private Wi-Fi, do not port-forward on router, do not use public Wi-Fi. For deployment, enable broker authentication and restrict exposure. Cloudflare WARP/VPN can interfere with local access; check routing if connection fails.

## 3. Configure private credentials
Create `firmware/include/WifiSecrets.h` by copying `WifiSecrets.example.h`. Set WIFI_SSID, WIFI_PASSWORD, MQTT_HOST (Windows Wi-Fi IPv4) and MQTT_PORT=1883. Add `firmware/include/WifiSecrets.h` to root `.gitignore`. NEVER commit it.

```powershell
Copy-Item firmware/include/WifiSecrets.example.h firmware/include/WifiSecrets.h
Add-Content .gitignore "`nfirmware/include/WifiSecrets.h"
```

Make sure ignore entry not duplicated; `git check-ignore firmware/include/WifiSecrets.h` should print file path.

## 4. Build, upload and monitor
In VS Code open the `firmware/` directory as PlatformIO project. Connect board to USB. Choose COM port if not autodetected (`pio device list` or `platformio device list`).

```powershell
cd D:\IOT-PROJECT\firmware
pio run
pio run --target upload
pio device monitor --baud 115200
```

If PlatformIO CLI `pio` isn't in PATH, use PlatformIO IDE's Build / Upload / Monitor buttons. Only one serial monitor process at a time.

Expected serial output:

```
[WIFI] connected IP=192.168.1.123
[MQTT] connected
[PUBLISH] OK {"device_id":"cane-01","distance_cm":45.0,...}
```

## 5. End-to-end verification
Ensure Spring Boot remains started in another terminal (run with DB_PASSWORD and `-Duser.timezone=Asia/Ho_Chi_Minh` from earlier G03/G04).
From project root:

```powershell
docker compose --env-file .env -f infra/compose.yaml exec -T postgres psql -U smartcane -d smartcane -c "SELECT id,device_id,distance_cm,received_at FROM telemetry WHERE device_id='cane-01' ORDER BY id DESC LIMIT 10;"
```

`cane-01` overlaps with earlier manual G03 test; **verify new timestamps and increasing IDs**, not just presence.

To inspect MQTT directly:

```powershell
docker compose --env-file .env -f infra/compose.yaml exec -T mosquitto mosquitto_sub -h localhost -p 1883 -t 'smartcane/cane-01/telemetry' -v
```

## 6. Reconnection acceptance
While board publishes, disconnect Wi-Fi temporarily (not the Windows machine hosting Docker). Restore and verify `[WIFI] connected` / `[MQTT] connected` and new telemetry. Alternatively stop and restart mosquitto container; verify device reconnects. Perform on trusted network. This firmware uses one-time MQTT LWT `offline` and retained `online` status; backend only subscribes telemetry/events at present.

## 7. Deliverables
- Screenshot of board + USB connection, Serial Monitor logs with local IP, MQTT connected, PUBLISH OK.
- PostgreSQL query with **new** timestamps/IDs from real ESP32.
- Test evidence of Wi-Fi/MQTT reconnect.
- Commit `firmware/platformio.ini`, `firmware/src/main.cpp`, `firmware/include/TelemetryData.h`, `firmware/include/WifiSecrets.example.h`, docs, and Compose port change; NEVER commit `WifiSecrets.h`, `.env`, tokens or `firmware/.pio`.
