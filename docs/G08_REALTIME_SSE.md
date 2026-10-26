# G08 - Realtime SSE

## Endpoint
`GET /api/devices/{deviceId}/stream` with `Accept: text/event-stream`

Events:
- `connected`: SSE connection established
- `status`: online, last_seen and status_source telemetry_freshness; online means the last telemetry is less than 15 seconds old, **not** an MQTT Last Will status
- `telemetry`: a persisted telemetry record; emitted after database transaction commits
- `incident`: a persisted SOS/fall/cancel record; emitted after database transaction commits
- `heartbeat`: server heartbeat every ~5 seconds

## Frontend
```js
const source = new EventSource('http://localhost:8081/api/devices/cane-01/stream');
source.addEventListener('telemetry', e => console.log('TELEMETRY', JSON.parse(e.data)));
source.addEventListener('incident', e => console.log('INCIDENT', JSON.parse(e.data)));
source.addEventListener('status', e => console.log('STATUS', JSON.parse(e.data)));
source.onerror = e => console.warn('SSE temporarily disconnected; browser retries', e);
// source.close() when leaving dashboard.
```

Use G07 REST endpoints to load initial history and backfill after reconnection. SSE does not replay historical events.
CORS permits Live Server origins localhost:5500 and 127.0.0.1:5500. `EventSource` supports automatic retry.
The streaming endpoint is currently unauthenticated, suitable for a LAN demo only. Add auth, per-device access control, rate limits before internet deployment.

## Test without ESP32
Start Docker and Spring Boot. Open another PowerShell window:
```powershell
curl.exe -N -H 'Accept: text/event-stream' 'http://localhost:8081/api/devices/cane-01/stream'
```
Keep it open. In a second terminal, publish MQTT JSON (mosquitto container):
```powershell
'{"device_id":"cane-01","distance_cm":82.0,"gps_valid":false,"water_detected":false,"light_level":1200,"led_on":false}' | docker compose --env-file .env -f infra/compose.yaml exec -T mosquitto mosquitto_pub -h localhost -p 1883 -t smartcane/cane-01/telemetry -s
```
SOS test: choose a NEW event_id each time, e.g. sos-g08-001:
```powershell
'{"device_id":"cane-01","event_id":"sos-g08-001","type":"SOS","gps_valid":false}' | docker compose --env-file .env -f infra/compose.yaml exec -T mosquitto mosquitto_pub -h localhost -p 1883 -t smartcane/cane-01/events -s
```
Expected `event: telemetry` and `event: incident` SSE output; status turns online after a telemetry record and offline after ~15s of inactivity (status scanned every ~5s).
