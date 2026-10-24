# GPIO ESP32 DevKit / CH340 30-pin — DỰ THẢO CẦN DUYỆT

| Chức năng | GPIO ESP32 | Ghi chú |
|---|---|---|
| HC-SR04 trước TRIG / ECHO | 25 / 26 | Echo 5V phải qua chia áp/chuyển mức |
| Driver motor rung | 27 | Qua transistor/MOSFET + diode, tuyệt đối không nối motor trực tiếp GPIO |
| GPS NEO-6M TX → ESP32 RX2 | 16 | UART2 RX |
| GPS NEO-6M RX ← ESP32 TX2 | 17 | UART2 TX; chỉ cần nếu cấu hình GPS |
| MPU6050 SDA / SCL | 21 / 22 | I2C; kiểm tra nguồn module |
| Nút SOS | 13 | `INPUT_PULLUP`, nối nút đến GND |
| Buzzer driver | 14 | Buzzer có dòng lớn phải qua transistor |
| Water Sensor analog | 35 | Chỉ input; bảo đảm tín hiệu 0–3.3V |
| LDR module analog | 34 | Chỉ input; bảo đảm tín hiệu 0–3.3V |
| LED thường | 32 | Qua điện trở hạn dòng, ví dụ 220Ω |
| HC-SR04 hướng xuống TRIG / ECHO | 18 / 19 | OPTIONAL: dễ bị sai lệch vì chuyển động gậy |

**Mức điện áp:** GPIO ESP32 **không chịu 5V**. Echo 5V phải dùng mạch chuyển mức phù hợp hoặc chia áp. Phương án gọn được khuyến nghị: **1kΩ từ Echo đến GPIO + 1.8kΩ từ GPIO xuống GND**, đầu ra ~3.21V khi Echo 5V. Nếu chỉ có 1kΩ và 2.2kΩ: có thể mắc **hai điện trở 1kΩ nối tiếp** phía Echo và **một điện trở 2.2kΩ** từ GPIO xuống GND → ~2.62V (logic HIGH trên nhiều ESP32, nhưng cần xác nhận mức nhận HIGH và đo thực tế). Không dùng 1k phía trên + 2k2 phía dưới vì tạo ~3.44V, vượt 3.3V.

**Nguồn:** Dùng USB trong giai đoạn thử. Khi dùng 18650 → TP4056 có bảo vệ → boost chỉnh đúng 5V, kiểm tra dòng và chân VIN/5V thực tế của board trước khi cấp. Không nối 5V vào chân 3V3.

**Quản trị:** Bảng này là phân bổ GPIO dự thảo v1. Kiểm tra bằng sơ đồ chân đúng revision board và điện áp thực; thành viên đề xuất đổi chân qua PR cho Giang duyệt. Không gắn trực tiếp tải cảm ứng (motor) vào GPIO.
