# Báo Cáo Kỹ Thuật & Nghiệm Thu — Module của Đạt (Tuần 1)
**Học phần:** Thực tập / Bài tập lớn IoT — Học viện Công nghệ Bưu chính Viễn thông (PTIT)  
**Dự án:** Gậy Thông Minh Hỗ Trợ Người Khiếm Thị (Smart Cane IoT)  
**Thành viên thực hiện:** Đạt  
**Mã công việc đảm nhận:** Đ01, Đ02, Đ04, Đ06  
**Nhánh Git:** `feature/dashboard-environment`

---

## 1. Tổng quan phân công & Trách nhiệm (Theo bảng phân công)
- **Phần cứng & Firmware (Đ04):**
  - Cảm biến tiếp xúc nước (Water Sensor dạng bản mạch): phát hiện khi gậy chạm vũng nước sâu hoặc đường ngập úng.
  - Cảm biến ánh sáng quang trở (LDR Module): nhận diện cường độ ánh sáng môi trường.
  - Đèn LED ban đêm (Night LED qua trở hạn dòng 220Ω): tự động bật sáng khi trời tối để cảnh báo phương tiện giao thông.
  - Viết module C++ độc lập: `firmware/include/EnvironmentModule.h` và `firmware/src/EnvironmentModule.cpp` tuân thủ hợp đồng `docs/MODULE_INTERFACE.md`.
- **Giao diện Web Dashboard & Bản đồ (Đ01, Đ02):**
  - Dựng giao diện Web Dashboard độc lập (HTML5, Vanilla CSS Glassmorphism, JavaScript ES6).
  - Tích hợp bản đồ Leaflet theo dõi vị trí GPS, vòng tròn hàng rào an toàn (Geofence) 350m tại PTIT Hà Đông.
  - Thiết kế bộ giả lập 8 kịch bản kiểm thử (Scenario Simulator) và sinh chuỗi JSON Telemetry theo đúng hợp đồng MQTT (`docs/MQTT_CONTRACT.md`).
- **Mô phỏng Wokwi (Đ04):**
  - Tạo sơ đồ mạch `simulation/dat/diagram.json` và code `simulation/dat/sketch.ino`.

---

## 2. Phần cứng & Mô phỏng Wokwi (Đ04)

### 2.1. Bảng phân bổ chân GPIO (Khớp docs/PINOUT.md)
| Linh kiện | Chân linh kiện | Chân ESP32 | Loại chân | Ghi chú an toàn & kỹ thuật |
|---|---|---|---|---|
| Water Sensor | Signal (AO) | **GPIO 35** | ADC1_CH7 (Input only) | Đo điện áp 0–3.3V, an toàn tuyệt đối khi bật Wi-Fi |
| LDR Module | Analog Out (AO) | **GPIO 34** | ADC1_CH6 (Input only) | Đo điện áp 0–3.3V từ cầu phân áp quang trở |
| LED Ban đêm | Anode (+) | **GPIO 32** | Digital Output | Nối tiếp qua điện trở hạn dòng 220Ω xuống GND |

### 2.2. Bảng dữ liệu ADC và Ngưỡng logic
- **Dải ADC ESP32:** 12-bit (0 – 4095), tương ứng 0V – 3.3V.
- **Cảm biến nước (GPIO 35):**
  - Trạng thái Khô ráo: ADC trong khoảng `0 – 800` &rarr; `water_detected = false`.
  - Trạng thái Chạm nước: ADC $\ge$ `1500` &rarr; `water_detected = true` (kích hoạt cảnh báo rung trên gậy và cờ dữ liệu gửi lên Web).
- **Cảm biến quang trở LDR (GPIO 34) & LED (GPIO 32) với trễ Hysteresis:**
  - Ngưỡng bật đèn: Khi ánh sáng giảm xuống dưới mức `1600 ADC` (Trời sẩm tối) &rarr; `led_on = true`, xuất mức HIGH ra GPIO 32.
  - Ngưỡng tắt đèn: Khi ánh sáng vượt mức `2100 ADC` (Trời sáng rõ) &rarr; `led_on = false`, xuất mức LOW ra GPIO 32.
  - Vùng đệm trễ `1600 – 2100 ADC`: Giữ nguyên trạng thái trước đó, giúp triệt tiêu hiện tượng đèn LED bị chớp nháy (flickering) khi ánh sáng chập chờn.

### 2.3. Hướng dẫn chạy mô phỏng Wokwi
1. Truy cập [Wokwi ESP32](https://wokwi.com/projects/new/esp32).
2. Dán nội dung file `simulation/dat/diagram.json` vào tab `diagram.json`.
3. Dán mã nguồn `simulation/dat/sketch.ino` vào tab `sketch.ino`.
4. Bấm **Play**:
   - Xoay biến trở Water Sensor giả lập &rarr; Quan sát cờ `water_detected` chuyển `true` khi vượt 1500.
   - Kéo độ sáng LDR &rarr; Đèn LED trên GPIO 32 tự động sáng khi tối và tắt khi sáng.

---

## 3. Kiến trúc Module Firmware C++ (`EnvironmentModule`)

Tuân thủ nghiêm ngặt quy ước hợp đồng tại `docs/MODULE_INTERFACE.md`:
- **Độc lập, không chứa `setup()` hay `loop()`**: Chỉ cung cấp các hàm phương thức:
  - `begin()`: Khởi tạo GPIO `pinMode`.
  - `update(unsigned long nowMs)`: Cập nhật non-blocking, chu kỳ lấy mẫu 100ms có bộ lọc trung bình trượt (Moving Average 5 mẫu) khử nhiễu ADC.
  - `getData()`: Trả về struct snapshot `EnvironmentData` gồm:
    ```cpp
    struct EnvironmentData {
        bool water_detected;   // Chạm nước (true/false)
        int light_level;       // Mức sáng ADC (0 - 4095)
        bool led_on;           // Trạng thái LED (true/false)
        int raw_water_adc;     // ADC thô phục vụ debug
    };
    ```
- **Không tự ý Publish MQTT**: Việc đóng gói và gửi dữ liệu do `MqttService` của Giang quản lý tại `main.cpp`.

---

## 4. Web Dashboard & Hệ Thống Giám Sát (Đ01, Đ02)

### 4.1. Công nghệ & Kiến trúc
- **Giao diện:** HTML5 ngữ nghĩa, CSS3 hiện đại theo phong cách **Dark Glassmorphism** (nền tối `#0a0e17`, hiệu ứng mờ kính `backdrop-filter`, viền phát sáng cyan/emerald/rose).
- **Typography:** Font chữ chuẩn công nghệ Google Fonts *Plus Jakarta Sans* và *Fira Code*.
- **Bản đồ Leaflet:**
  - Vị trí định vị gậy thông minh với icon gậy phát xung radar hoạt họa.
  - Hàng rào địa lý ảo (**Geofence**) bán kính 350m tại khuôn viên PTIT Hà Đông. Tự động chuyển màu đỏ và cảnh báo khi tọa độ nằm ngoài bán kính an toàn.
  - Đường vết di chuyển thời gian thực (Breadcrumb trail).
- **Hệ thống cảnh báo âm thanh:** Tích hợp bộ tổng hợp âm thanh bằng **Web Audio API** (tạo tiếng bíp cảnh báo thực tế khi có SOS, chạm nước hoặc nghi ngờ ngã, không cần phụ thuộc file MP3 ngoại vi).

### 4.2. Bộ giả lập 8 Kịch bản (Scenario Simulator)
Đạt đã tích hợp sẵn bảng điều khiển giả lập phục vụ báo cáo và kiểm thử trước khi ghép mạch thật:
1. **🟢 Kịch bản 1 - Bình thường:** Đường thông thoáng, khoảng cách >150cm (an toàn), trời sáng, pin 94%.
2. **🟡 Kịch bản 2 - Vật cản gần:** Khoảng cách 32.5cm &rarr; Báo động đỏ, mô tơ rung mức 3 liên tục.
3. **🌊 Kịch bản 3 - Chạm vũng nước:** Cảm biến nước đạt ADC 2650 &rarr; Biểu ngữ cảnh báo nước ngập bung ra.
4. **🌙 Kịch bản 4 - Trời tối:** LDR ADC 820 &rarr; Tự động kích hoạt LED ban đêm.
5. **🚨 Kịch bản 5 - Nhấn nút SOS:** Biểu ngữ khẩn cấp đỏ rực, âm thanh báo động vang lên, gửi sự kiện SOS.
6. **⚠️ Kịch bản 6 - Nghi ngờ gậy đổ:** MPU6050 phát hiện gậy ngã, còi đếm lùi kích hoạt.
7. **📍 Kịch bản 7 - Ra khỏi Geofence:** Dịch chuyển tọa độ vượt bán kính 350m &rarr; Cảnh báo rời vùng an toàn.
8. **❌ Kịch bản 8 - Mất kết nối:** Mô phỏng cơ chế Last Will and Testament (LWT) khi thiết bị offline.

---

## 5. Hướng dẫn chạy thử nghiệm & Nghiệm thu
1. Khởi động Web Server cục bộ:
   ```powershell
   python -m http.server 5500 -d frontend
   ```
2. Mở trình duyệt truy cập: `http://localhost:5500`
3. Thử nghiệm các nút kịch bản ở bảng dưới cùng bên phải và quan sát bản đồ, thẻ chỉ số, banner cảnh báo và chuỗi JSON cập nhật tự động.

---

## 6. Trạng thái bàn giao (Checklist Đ01, Đ02, Đ04, Đ06)
- [x] **Đ01:** Web khung responsive, cấu trúc wireframe hoàn thiện.
- [x] **Đ02:** Bản đồ Leaflet, marker gậy, geofence, 8 kịch bản giả lập, hoạt động độc lập 100%.
- [x] **Đ04:** Mô phỏng Wokwi (`diagram.json`, `sketch.ino`), module C++ (`EnvironmentModule.h/cpp`), bảng ADC.
- [x] **Đ06:** Tài liệu báo cáo chi tiết, template và các số liệu nghiệm thu.
