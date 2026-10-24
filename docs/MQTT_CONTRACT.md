# MQTT / JSON Contract — v0 (nhóm chốt trước khi tích hợp)

- Prefix: `smartcane/{device_id}`. `device_id` mặc định `cane-01` (không chứa dấu `/`).
- `smartcane/cane-01/telemetry` — QoS 0, mỗi 1–5 giây tùy cảm biến, retain=false.
- `smartcane/cane-01/events` — QoS 1, retain=false. Có `event_id` để chống trùng khi broker phát lại.
- `smartcane/cane-01/status` — online/offline với LWT; có thể retain=true sau khi kiểm thử.
- Timestamp chuẩn UTC ISO8601. **Dữ liệu thật có thể thiếu GPS và thời gian UTC khi không đồng bộ**: backend không tự cho rằng tọa độ luôn tồn tại.

## Telemetry mẫu (dữ liệu giả)
```json
{"device_id":"cane-01","timestamp":"2026-10-24T12:00:00Z","distance_cm":83.5,"latitude":null,"longitude":null,"gps_valid":false,"water_detected":false,"light_level":1200,"led_on":false,"battery_voltage":null}
```

## Event mẫu
```json
{"event_id":"cane-01-000001","device_id":"cane-01","timestamp":"2026-10-24T12:00:10Z","type":"SOS","latitude":null,"longitude":null,"gps_valid":false}
```

Các kiểu sự kiện đề xuất: `SOS`, `CANE_FALL_SUSPECTED`, `CANE_FALL_ALERT`, `CANCEL`. ESP32 phát sự kiện, **chỉ backend** chịu trách nhiệm quyết định gửi Telegram. Không dùng `FALL_DETECTED` (khẳng định người ngã).

**Cần quyết định trong họp:** cơ chế retry/idempotency, nhận diện event_id sau reboot, lưu timezone, `distance_cm` khi timeout (null), giới hạn GPS stale, quyền truy cập MQTT production. Broker scaffold chỉ sử dụng local dev, chưa có authentication.

## Quy ước kiểm thử / tiến hóa
- `obstacle_level` tùy chọn enum `SAFE|WARNING|DANGER|UNKNOWN`; cần cập nhật schema và tests đồng thời nếu bật.
- JSON telemetry gồm giá trị đo, không tùy ý đưa biến thuật toán vào contract.
- Null cho sensor chưa có / lỗi; `timestamp` thiếu đồng bộ thì cần đánh dấu rõ ràng, không tự giả mạo timestamp UTC.
- Thay đổi schema/topic/QoS là thay đổi chung: mở PR, cập nhật test JSON, backend, frontend và firmware cùng lúc.
