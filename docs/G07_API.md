# G07 – REST API bàn giao cho Đạt

## Endpoints (backend chạy trên http://localhost:8081)

- `GET /api/devices/cane-01/telemetry/latest`: JSON telemetry object mới nhất; HTTP 404 nếu chưa có.
- `GET /api/devices/cane-01/telemetry?from=2026-10-26T00:00:00Z&to=2026-10-27T00:00:00Z&limit=100`: danh sách telemetry theo khoảng UTC, mới nhất trước, tối đa 500.
- `GET /api/devices/cane-01/incidents?limit=20`: danh sách sự kiện (SOS/FALL_ALERT...) mới nhất trước, tối đa 100.
- `GET /api/devices/cane-01/status`: `{device_id,online,last_seen,status_source}`.

`status.online` là **suy luận từ thời điểm telemetry gần nhất trong 15 giây**, không phải trạng thái MQTT Last Will; trạng thái MQTT chính xác hơn thuộc G08. `limit < 1` trả HTTP 400; `from > to` trả HTTP 400.

JSON các entity JPA dùng tên trường Java camelCase như `deviceId`, `distanceCm`, `receivedAt` (riêng status dùng snake_case). Frontend Đạt cần lưu ý khác biệt này; chưa chuẩn hóa DTO response ở G07.

## Kiểm thử PowerShell

```powershell
cd D:\IOT-PROJECT\backend
$env:DB_PASSWORD = Read-Host 'Nhap mat khau PostgreSQL'
$env:JAVA_TOOL_OPTIONS = '-Duser.timezone=UTC'
mvn clean test
mvn spring-boot:run
```

Mở terminal khác:

```powershell
Invoke-RestMethod http://localhost:8081/api/devices/cane-01/telemetry/latest
Invoke-RestMethod 'http://localhost:8081/api/devices/cane-01/incidents?limit=10'
Invoke-RestMethod 'http://localhost:8081/api/devices/cane-01/status'
Invoke-RestMethod 'http://localhost:8081/api/devices/cane-01/telemetry?from=2026-10-26T00:00:00Z&to=2026-10-27T00:00:00Z&limit=10'
```

Lưu ý: đây là patch thay thế `ReadApi.java` và `TelemetryRepository.java` đã có từ G03–G04, **không giải nén đè cả backend**. Không đổi schema database, hạ tầng hay firmware.
