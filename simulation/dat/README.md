# Wokwi Simulation — Module Môi Trường & Đèn (Đạt)

## 1. Giới thiệu Module
Module do **Đạt** phụ trách bao gồm:
1. **Cảm biến tiếp xúc nước (Water Sensor):** Phát hiện gậy chạm nước ngập / vũng nước trên đường đi để cảnh báo người dùng.
2. **Cảm biến quang trở (LDR Module):** Đo cường độ ánh sáng môi trường xung quanh.
3. **Đèn LED ban đêm (Night LED):** Tự động kích hoạt khi trời tối giúp tăng độ nhận diện cho người khiếm thị khi đi lại vào ban đêm.

---

## 2. Sơ đồ đấu nối (Wiring Pinout)
Khớp hoàn toàn với quy ước chung của nhóm tại `docs/PINOUT.md`:

| Linh kiện | Chân linh kiện | Chân ESP32 | Ghi chú kỹ thuật |
|---|---|---|---|
| **Water Sensor (giả lập bằng Potentiometer)** | VCC | 3V3 | Nguồn 3.3V từ ESP32 |
| | GND | GND | Nối mass chung |
| | SIG / Analog Out | **GPIO 35** (ADC1_CH7) | Chân Input-only trên ESP32, an toàn khi bật Wi-Fi |
| **LDR Module** | VCC | 3V3 | Nguồn 3.3V |
| | GND | GND | Nối mass chung |
| | AO (Analog Out) | **GPIO 34** (ADC1_CH6) | Chân Input-only trên ESP32, an toàn khi bật Wi-Fi |
| **LED Ban đêm** | Anode (+) | **GPIO 32** | Nối qua điện trở hạn dòng 220Ω |
| | Cathode (-) | GND | Nối mass chung |

---

## 3. Bảng dữ liệu ADC và Ngưỡng logic (Threshold Matrix)

### Cảm biến nước (GPIO 35):
- Dải ADC ESP32: 0 - 4095 (12-bit).
- **Trạng thái Khô ráo:** ADC đo được: `0 - 800` (Điện trở cách điện cao giữa 2 cực) -> `water_detected = false`.
- **Trạng thái Chạm nước:** ADC đo được: `> 1500` (Nước dẫn điện làm giảm trở kháng) -> `water_detected = true`, kích hoạt cờ cảnh báo `WATER_WARNING`.

### Cảm biến ánh sáng LDR (GPIO 34) & LED (GPIO 32):
- Để tránh hiện tượng LED nhấp nháy liên tục (flickering) khi ánh sáng ở ngưỡng biên, thuật toán sử dụng **bộ trễ Hysteresis**:
  - Khi ánh sáng giảm: `ADC < 1600` (Trời tối dần) -> **Bật LED** (`led_on = true`).
  - Khi ánh sáng tăng: `ADC > 2100` (Trời sáng rõ) -> **Tắt LED** (`led_on = false`).
  - Vùng `1600 - 2100`: Giữ nguyên trạng thái trước đó.

---

## 4. Cách chạy mô phỏng trên Wokwi
1. Mở trang [Wokwi ESP32 Simulator](https://wokwi.com/projects/new/esp32).
2. Copy toàn bộ nội dung file `diagram.json` vào tab `diagram.json` trên Wokwi.
3. Copy toàn bộ mã nguồn file `sketch.ino` vào tab `sketch.ino`.
4. Nhấn nút **Play (Start Simulation)**:
   - Kéo núm chiết áp Water Sensor: quan sát Serial Monitor khi vượt ngưỡng 1500 sẽ thấy JSON đổi `"water_detected": true`.
   - Kéo thanh trượt ánh sáng trên LDR: quan sát đèn LED màu trắng trên chân GPIO 32 tự động bật khi kéo sang vùng tối và tự tắt khi sáng.

---

## 5. Mẫu dữ liệu đầu ra Serial (Chuẩn hóa MQTT Telemetry)
```json
{"device_id":"cane-01","water_detected":true,"water_adc":1890,"light_level":1200,"led_on":true,"status":"WATER_WARNING"}
```

---

## 6. Checklist nghiệm thu (Đ04)
- [x] Sơ đồ mạch `diagram.json` chuẩn chân GPIO 34, 35, 32.
- [x] Mã nguồn `sketch.ino` đọc ADC có lọc nhiễu + Hysteresis cho đèn.
- [x] Dữ liệu JSON khớp định dạng hợp đồng `docs/MQTT_CONTRACT.md`.
- [x] Đã đóng gói thành module C++ độc lập (`EnvironmentModule.h/cpp`) sẵn sàng cho Giang tích hợp vào `main.cpp`.
