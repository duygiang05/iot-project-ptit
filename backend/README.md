# Backend — Giang

**Trạng thái:** chưa có project Spring Boot, thư mục này là vị trí phát triển; không được coi là backend chạy được.

Khởi tạo Java 17 / Maven / Spring Boot; các dependency đề xuất: Web, Validation, Data JPA, PostgreSQL Driver, MQTT client (chọn một thư viện có hỗ trợ broker), WebSocket nếu dùng. Thiết kế package: `config/`, `mqtt/`, `controller/`, `service/`, `entity/`, `repository/`, `dto/`.

Việc đầu tiên: subscribe `smartcane/+/telemetry` và `smartcane/+/events`, parse JSON mẫu, in log. Tiếp theo database schema/API, chống duplicate `event_id`, Telegram server-side. Bảo vệ token qua env, không commit mật khẩu.

Dùng `tests/` làm nguồn contract; khi chưa có ESP32 thật có thể dùng MQTTX/CLI publish dữ liệu giả.
