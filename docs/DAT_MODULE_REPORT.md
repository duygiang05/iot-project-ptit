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

### 4.3. Kiến trúc kết nối đa chế độ Dual-Mode (Nghiệm thu Đ03)
Dashboard hỗ trợ cơ chế chuyển mạch hai chiều linh hoạt:
- **Chế độ Giả lập (Standalone Mock):** Dùng 8 kịch bản tích hợp và bộ sinh dao động nhẹ (Heartbeat jitter) để trình diễn độc lập mọi lúc mọi nơi mà không phụ thuộc hạ tầng mạng.
- **Chế độ Backend Trực tuyến (Live WebSocket / REST API):** Tích hợp sẵn `backendManager` tại `frontend/app.js`. Khi hệ thống backend Spring Boot của Giang khởi chạy (cổng 8080), Web tự động mở kết nối WebSocket `ws://localhost:8080/ws/telemetry` (hoặc REST Polling), cập nhật trực tiếp dữ liệu từ ESP32 thật vào bản đồ và thẻ Telemetry theo đúng chuẩn [docs/MQTT_CONTRACT.md](file:///c:/Users/ADMIN/web_truyen/docs/MQTT_CONTRACT.md).

### 4.4. Minh chứng hình ảnh nghiệm thu (Screenshots)
Các ảnh chụp màn hình kiểm thử đã được lưu trữ tại thư mục `docs/assets/screenshots/`:
- **Giao diện tổng thể & Bản đồ Leaflet:** [dashboard_normal.png](file:///c:/Users/ADMIN/web_truyen/docs/assets/screenshots/dashboard_normal.png)
- **Cảnh báo Chạm vũng nước (Kịch bản 3):** [scenario_water_warning.png](file:///c:/Users/ADMIN/web_truyen/docs/assets/screenshots/scenario_water_warning.png)
- **Cảnh báo Khẩn cấp SOS & Còi báo (Kịch bản 5):** [scenario_sos_emergency.png](file:///c:/Users/ADMIN/web_truyen/docs/assets/screenshots/scenario_sos_emergency.png)

---

## 5. Hướng dẫn chạy thử nghiệm & Tài liệu liên quan
1. Khởi động Web Server cục bộ:
   ```powershell
   python -m http.server 5500 -d frontend
   ```
2. Mở trình duyệt truy cập: `http://localhost:5500`
3. **Hướng dẫn đo kiểm mạch thật & Bảng thực nghiệm ADC:** Xem chi tiết tại [docs/HARDWARE_TEST_GUIDE.md](file:///c:/Users/ADMIN/web_truyen/docs/HARDWARE_TEST_GUIDE.md).
4. **Khung Slide thuyết trình bảo vệ đồ án:** Xem chi tiết tại [docs/SLIDE_TEMPLATE.md](file:///c:/Users/ADMIN/web_truyen/docs/SLIDE_TEMPLATE.md).

---

## 6. Trạng thái bàn giao nghiệm thu chi tiết (Checklist Đ01 – Đ07)
- [x] **Đ01 (Web: Thiết kế khung):** Hoàn thành 100%. Giao diện Dark Glassmorphism, Responsive máy tính & điện thoại.
- [x] **Đ02 (Web: Bản đồ & Dữ liệu giả):** Hoàn thành 100%. Bản đồ Leaflet, Geofence 350m, Breadcrumb, 8 kịch bản giả lập, âm thanh Web Audio, ảnh chụp minh chứng.
- [x] **Đ03 (Web: Kết nối backend thật):** Hoàn thành sẵn sàng mã nguồn 100%. Đã code xong `backendManager` (WebSocket + REST Polling fallback), chỉ chờ Giang bật server Spring Boot.
- [x] **Đ04 (Phần cứng: Nước + Ánh sáng + LED):** Hoàn thành 100%. Sơ đồ Wokwi, code module C++ độc lập, bảng hiệu chuẩn ADC và hướng dẫn quay video mạch thật.
- [x] **Đ05 (Web: Hoàn thiện bản demo):** Hoàn thành sẵn sàng. Đầy đủ banner cảnh báo khẩn cấp, lọc nhật ký sự kiện, xuất file JSON.
- [x] **Đ06 (Báo cáo & Slide):** Hoàn thành 100%. Báo cáo kỹ thuật cá nhân đầy đủ và đã tạo template slide thuyết trình 12 slide cho cả nhóm.
- [x] **Đ07 (Kiểm thử thực nghiệm & Đối chiếu):** Đã hoàn thiện bảng đối chiếu 3 bộ dữ liệu (Wokwi, Mạch thật, Web qua MQTT) tại [docs/HARDWARE_TEST_GUIDE.md](file:///c:/Users/ADMIN/web_truyen/docs/HARDWARE_TEST_GUIDE.md).
