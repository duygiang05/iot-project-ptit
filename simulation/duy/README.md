# Duy — D01/D02: MPU6050, nghi ngờ gậy đổ, SOS và còi

## Mở project trên Wokwi

1. Tạo project mới loại **ESP32** tại Wokwi.
2. Thay nội dung `sketch.ino` bằng file cùng tên trong folder này.
3. Mở `diagram.json` trong editor, thay bằng sơ đồ cùng tên trong folder này.
4. `libraries.txt` đã ghi dependency (không cần thư viện ngoài). Bấm Start Simulation và xem Serial Monitor ở 115200 baud.

Nếu Serial báo `MPU6050: NOT FOUND`, kiểm tra dây I2C trong sơ đồ: `D21 -> SDA`, `D22 -> SCL`, `3V3 -> VCC`, `GND -> GND`. Nút SOS vẫn hoạt động riêng trên GPIO13.

- Nhấn giữ nút GPIO13 ít nhất 3 giây: nhận đúng một `[EVENT] SOS`; còi bật trong lúc nhấn và chirp khi gửi SOS.
- Nghiêng MPU6050 vượt 60° đồng thời tạo gia tốc thấp hơn 0.55g hoặc cao hơn 1.8g: sau 600ms sẽ có `[EVENT] CANE_FALL_SUSPECTED`.
- Trong 10 giây sau nghi ngờ, nhấn nút: `[EVENT] CANCEL`; để hết thời gian: `[EVENT] CANE_FALL_ALERT`.
- Sự kiện chỉ là dấu hiệu gậy có thể đổ, không xác nhận người dùng bị ngã. Các ngưỡng là giá trị demo, cần hiệu chỉnh trên gậy thật.

Sketch Wokwi là file độc lập, có thể chạy mà không cần thêm `SafetyModule.h/.cpp`. Chưa có link/ảnh/log chạy mô phỏng trong repo; sau khi chạy, lưu minh chứng tại đây. Mô phỏng không thay thế đo kiểm trên mạch thật.
