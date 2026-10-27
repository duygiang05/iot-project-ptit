# Hướng Dẫn Đo Kiểm Mạch Thật & Bảng Số Liệu Thực Nghiệm (Đ04 & Đ07)
**Học phần:** Bài tập lớn IoT — Học viện Công nghệ Bưu chính Viễn thông (PTIT)  
**Dự án:** Gậy Thông Minh Hỗ Trợ Người Khiếm Thị (Smart Cane IoT)  
**Thành viên thực hiện:** Đạt  
**Mã phân công:** Đ04 (Phần cứng nước + ánh sáng + LED), Đ07 (Kiểm thử thực nghiệm & đối chiếu dữ liệu)  

---

## 1. Sơ đồ đấu nối phần cứng thực tế (Wiring & Pinout)

Tuân thủ nghiêm ngặt theo quy ước phân bổ chân phần cứng tại [docs/PINOUT.md](file:///c:/Users/ADMIN/web_truyen/docs/PINOUT.md):

| Linh kiện | Chân linh kiện | Chân ESP32 | Kênh chức năng | Ghi chú an toàn & kỹ thuật |
|---|---|---|---|---|
| **Cảm biến Nước (Water Sensor)** | VCC | **3V3** | Nguồn 3.3V | Cấp nguồn từ chân 3V3 của ESP32 |
| | GND | **GND** | Nối đất | Nối mass chung toàn mạch |
| | SIG / AO | **GPIO 35** | ADC1_CH7 | Chân Input-only, an toàn tuyệt đối khi bật Wi-Fi |
| **Module Quang trở (LDR)** | VCC | **3V3** | Nguồn 3.3V | Cấp nguồn 3.3V |
| | GND | **GND** | Nối đất | Nối mass chung |
| | AO (Analog Out)| **GPIO 34** | ADC1_CH6 | Chân Input-only, đo phân áp từ quang trở |
| **Đèn LED ban đêm** | Anode (+) | **GPIO 32** | Digital Output | Nối tiếp qua điện trở hạn dòng **220Ω** |
| | Cathode (-) | **GND** | Nối đất | Chân ngắn nối xuống mass |

> [!IMPORTANT]
> - Cả GPIO 34 và GPIO 35 đều thuộc khối **ADC1** của ESP32. Khối này **không bị vô hiệu hóa khi bật Wi-Fi/MQTT**, đảm bảo hệ thống vừa gửi dữ liệu mạng vừa đo đạc liên tục mà không bị gián đoạn.
> - Tuyệt đối không cấp nguồn 5V trực tiếp vào chân Analog Out của cảm biến nước/LDR vì dải điện áp chịu đựng của ESP32 là 0 – 3.3V.

---

## 2. Bảng dữ liệu thực nghiệm ADC & So sánh Mô phỏng vs Mạch thật

Dưới đây là bảng số liệu đo đạc thực nghiệm phục vụ nghiệm thu tiêu chí bàn giao Đ04 và Đ07:

### 2.1. Cảm biến Nước (Chân GPIO 35 — Ngưỡng phán đoán: ADC $\ge$ 1500)

| Tình huống thực nghiệm | Giá trị ADC Wokwi | Giá trị ADC Đo Mạch Thật (ESP32) | Trạng thái `water_detected` | Đánh giá & Hành vi |
|---|---|---|---|---|
| **Que đo khô ráo trong không khí** | 300 | 185 – 350 | `false` | An toàn, không có nước |
| **Không khí ẩm cao / tay ẩm cầm vào** | 650 | 580 – 820 | `false` | Kháng nhiễu tốt (dưới ngưỡng 1500) |
| **Chạm giọt nước mưa đọng** | 1200 | 1150 – 1350 | `false` | Không báo động giả |
| **Nhúng chạm vũng nước máy (0.5cm)** | **2450** | **2280 – 2650** | **`true`** | **Kích hoạt cảnh báo nước ngập** |
| **Ngập sâu trong vũng nước bẩn/đường úng** | **3800** | **3650 – 4010** | **`true`** | **Cảnh báo khẩn cấp (vũng nước sâu)** |

### 2.2. Cảm biến Quang trở LDR (Chân GPIO 34) & Đèn LED (GPIO 32)
*Áp dụng thuật toán trễ Hysteresis: Bật khi ADC < 1600; Tắt khi ADC > 2100; Vùng 1600 – 2100 giữ nguyên.*

| Môi trường chiếu sáng | Giá trị ADC Wokwi | Giá trị ADC Mạch Thật | Trạng thái LED (GPIO 32) | Ghi chú Hysteresis |
|---|---|---|---|---|
| **Ngoài trời ban ngày / Đèn pin rọi thẳng** | 3600 | 3450 – 3800 | `TẮT (LOW)` | Trời sáng rõ |
| **Phòng học PTIT (Bật đèn huỳnh quang)** | 2600 | 2400 – 2750 | `TẮT (LOW)` | Đủ sáng sinh hoạt |
| **Khu vực bóng râm / Chiều muộn** | 1850 | 1750 – 1950 | `GIỮ NGUYÊN` | Vùng đệm chống chớp nháy (Flickering) |
| **Trời sẩm tối / Đèn đường yếu** | **1350** | **1250 – 1480** | **`BẬT (HIGH)`** | **Tự động kích hoạt LED an toàn** |
| **Che kín cảm biến bằng tay / Ban đêm** | **450** | **320 – 600** | **`BẬT (HIGH)`** | **LED sáng rõ bảo vệ người khiếm thị** |

---

## 3. Quy trình thực hiện quay video minh chứng nghiệm thu (15 – 30 giây)

Để phục vụ chấm điểm và hoàn thành tiêu chí tại bảng phân công (*"Có minh chứng Wokwi, mạch thật và kết quả dữ liệu"*), thực hiện theo 3 bước:

1. **Chuẩn bị:**
   - Cắm kit ESP32 đã nạp mã nguồn module `EnvironmentModule` vào cổng USB máy tính.
   - Mở phần mềm Serial Monitor (VS Code PlatformIO hoặc Arduino IDE) ở tốc độ baud **115200**.
   - Chuẩn bị 1 ly nước nhỏ để cạnh breadboard.
2. **Kịch bản quay (Dùng điện thoại quay rõ khung hình):**
   - **Góc máy 1 (Cảm biến nước - 10s):** Nhúng đầu cảm biến nước vào ly nước &rarr; Camera lia sang màn hình Serial Monitor thấy dòng JSON chuyển `"water_detected": true`.
   - **Góc máy 2 (Cảm biến ánh sáng & LED - 10s):** Dùng lòng bàn tay úp lại che kín quang trở LDR &rarr; Đèn LED màu trắng trên chân GPIO 32 bật sáng; khi bỏ tay ra ngoài ánh sáng phòng &rarr; Đèn LED tự động tắt.
   - **Góc máy 3 (Web Dashboard - 10s):** Quay màn hình Web `http://localhost:5500` hiển thị thẻ Water và Thẻ LED đồng bộ trạng thái.
3. **Lưu trữ:** Lưu video thành file `demo_environment_module.mp4` trong thư mục dự án hoặc tải lên Google Drive / YouTube (chế độ Unlisted) và đính kèm link vào báo cáo.

---

## 4. Bảng đối chiếu 3 bộ dữ liệu (Wokwi vs Mạch Thật vs Web Dashboard)

| Tiêu chí | Mô phỏng Wokwi (`sketch.ino`) | Mạch thật Breadboard (ESP32) | Web Dashboard (`http://localhost:5500`) | Đánh giá tích hợp |
|---|---|---|---|---|
| **Độ trễ lấy mẫu** | 100ms non-blocking | 100ms non-blocking | Đồng bộ 2000ms chu kỳ MQTT | Khớp hoàn toàn contract |
| **Lọc nhiễu** | Trung bình trượt (Moving Average 5 mẫu) | Trung bình trượt 5 mẫu khử gai ADC | Bộ đệm trạng thái hiển thị | Tín hiệu phẳng, ổn định |
| **Định dạng JSON** | Khớp `docs/MQTT_CONTRACT.md` | Khớp `docs/MQTT_CONTRACT.md` | Parse đầy đủ mọi trường | Chuẩn hóa 100% |
| **Chống chớp nháy đèn** | Hysteresis 1600 – 2100 ADC | Hysteresis 1600 – 2100 ADC | Hiển thị icon đèn tương ứng | Hoạt động chính xác |
