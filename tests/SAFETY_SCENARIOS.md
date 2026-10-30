# D05 safety payload and test cases

Use synthetic coordinates only. `safety-events.jsonl` contains contract-shaped events. Publish each line to `smartcane/cane-01/events` with MQTT QoS 1.

| Case | Input/action | Expected result |
|---|---|---|
| Fall suspected | Publish `CANE_FALL_SUSPECTED` | Stored as incident; no Telegram notification |
| Fall not cancelled | Publish `CANE_FALL_ALERT` with valid GPS | One incident and one Telegram with Maps URL |
| Fall cancelled | Publish `CANCEL` | Incident stored, no Telegram alert |
| SOS without GPS | Publish SOS with null coordinates and `gps_valid=false` | One Telegram stating GPS unavailable; no map URL |
| GPS valid | Publish SOS with valid coordinates and `gps_valid=true` | Telegram includes Maps link |
| Duplicate delivery | Republish identical `event_id` | One DB incident and one notification at most |
| Invalid GPS | Set `gps_valid=true` but omit coordinates | Backend rejects the payload |
| Invalid type/topic | Unknown event type or mismatched device/topic | Backend rejects the payload |

`d06_integration.py` runs repeated MQTT-to-API checks for SOS, fall alert, cancel, and missing GPS. It records MQTT-to-database latency in JSON/CSV output. Telegram API latency is emitted by the backend as `[TELEGRAM] ... end_to_end_ms=...`; collect that log separately. Offline/local-buzzer checks require the physical device and are documented in the D06 report template.
