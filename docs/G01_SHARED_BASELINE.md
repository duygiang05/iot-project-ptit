# G01 — Baseline kỹ thuật chung (cần nhóm duyệt trước khi khóa)

## Kiến trúc
ESP32 (C++ Arduino / PlatformIO) → MQTT qua Wi-Fi → Mosquitto → Spring Boot Java 17 → PostgreSQL 16 → REST/Web. Telegram do backend gửi ở giai đoạn sau.

## Phân công và quyền file
- Giang: `docs/` hợp đồng chung, `infra/`, `backend/`, `firmware/src/main.cpp`, `firmware/include/PinConfig.h`, MQTT service.
- Tùng: `simulation/tung/`, module obstacle và GPS trong `firmware/`.
- Duy: `simulation/duy/`, module safety trong `firmware/`.
- Đạt: `simulation/dat/`, module environment trong `firmware/`, `frontend/`.
- Chỉ thay đổi GPIO/JSON/topic khi PR được duyệt. Các ngưỡng thuật toán do người phụ trách đề xuất và kiểm thử.

## Contract đóng băng phiên bản v0
- Device ID ví dụ: `cane-01`, ký tự chữ/số/_/-.
- `smartcane/{device_id}/telemetry`: QoS 0, retain=false.
- `smartcane/{device_id}/events`: QoS 1, retain=false, `event_id` duy nhất xuyên reboot.
- `smartcane/{device_id}/status`: LWT online/offline sẽ triển khai/kiểm thử sau.
- `distance_cm` đơn vị cm; `latitude`/`longitude` độ; `light_level` ADC chưa hiệu chuẩn; `battery_voltage` V.
- Vắng dữ liệu biểu diễn `null`, không tự điền GPS hay timestamp giả.
- Event type: `SOS`, `CANE_FALL_SUSPECTED`, `CANE_FALL_ALERT`, `CANCEL`.
- Timestamp ISO 8601 UTC khi đã có clock hợp lệ; backend lưu thời gian nhận khi nguồn không gửi timestamp.

## Rào chắn kỹ thuật
- GPIO ESP32 không chịu được Echo 5 V từ HC-SR04; phải chia áp/chuyển mức.
- Motor rung qua transistor/MOSFET và diode, không cấp trực tiếp từ GPIO.
- Cảm biến nước chỉ nhận biết tiếp xúc nước; thuật toán MPU6050 chỉ nghi ngờ *gậy* đổ.
- Wokwi public MQTT không kết nối được broker `localhost` trên Windows; test end-to-end bằng broker chung có đường truy cập phù hợp, hoặc ESP32 thật và LAN.

## Các quyết định chưa khóa (cố ý)
Ngưỡng HC-SR04/rung, MPU6050, LDR, water, bộ lọc, giao diện UI, chân pin cuối cùng. Người phụ trách đề xuất, đo kiểm; Giang cập nhật tài liệu sau khi cả nhóm xác nhận.

## Tiêu chí G01 Done
[ ] 4 thành viên xác nhận hiểu trách nhiệm; [ ] GPIO đã review với board thật; [ ] contract JSON/topic đã duyệt; [ ] cả nhóm clone repo và mở PR thử thành công.
