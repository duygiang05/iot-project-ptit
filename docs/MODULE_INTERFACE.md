# MODULE INTERFACE — QUY ƯỚC TÍCH HỢP

**Hợp đồng bắt buộc về hành vi:** mỗi module có `begin()` (khởi tạo), `update(nowMs)` (cập nhật nhanh, không chặn), `getData()` (trả snapshot chỉ chứa dữ liệu có hợp đồng), và cơ chế lấy sự kiện (`popEvent()` hoặc callback/queue theo mẫu Giang chốt trong PR giao diện).

Tên hàm được thống nhất trong PR giao diện module trước khi tích hợp. Các module **không tự định nghĩa `setup()` / `loop()`** trong `firmware/src/` và không trực tiếp publish MQTT hoặc gửi Telegram; ngoại lệ MqttService (Giang) chuyên trách kết nối/truyền.

## DTO thống nhất (bản hợp đồng khái niệm)
- `ObstacleData`: `distance_cm` (float hoặc null khi timeout), `obstacle_level` (SAFE/WARNING/DANGER/UNKNOWN).
- `GpsData`: `latitude`, `longitude` (number/null), `gps_valid` (boolean).
- `EnvironmentData`: `water_detected` (boolean/null), `light_level` (number/null), `led_on` (boolean).
- `SafetyEvent`: `event_id`, `type`, `timestamp`, tọa độ nếu có. Type: `SOS`, `CANE_FALL_SUSPECTED`, `CANE_FALL_ALERT`, `CANCEL`.

**Tách biệt:** `getData()` chỉ trả dữ liệu; main/MqttService mới serialize theo `docs/MQTT_CONTRACT.md`. Các ngưỡng/thuật toán do owner tự thử nghiệm ở file cấu hình riêng hoặc các biến nội bộ; khi đổi hành vi phải kèm minh chứng. Không dùng `delay()` dài gây chậm SOS/MQTT.

## Quy trình ghép
1. Module owner nộp sketch Wokwi + test log.
2. Chuyển `.ino` thành `.h/.cpp`, bảo đảm firmware build khi có module.
3. Giang review và tích hợp `main.cpp` theo một PR tích hợp riêng.
4. Test logic local + test thực tế; chỉ merge `main` khi smoke test đạt.
