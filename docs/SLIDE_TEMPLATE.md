# SLIDE BÁO CÁO THUYẾT TRÌNH BÀI TẬP LỚN IOT (PTIT)
**Đề tài:** Gậy Thông Minh Hỗ Trợ Người Khiếm Thị (Smart Cane IoT)  
**Nhóm sinh viên thực hiện:**
1. **Giang** — Kiến trúc hệ thống, Backend Spring Boot, MQTT Broker, PostgreSQL, Hạ tầng Docker
2. **Tùng** — Cảm biến siêu âm HC-SR04, Mô tơ rung cảnh báo, Module GPS NEO-6M, Nguồn pin
3. **Duy** — Cảm biến góc nghiêng MPU6050 (Gậy đổ), Nút bấm khẩn cấp SOS, Còi Buzzer
4. **Đạt** — Giao diện Web Dashboard (Leaflet, Geofence), Cảm biến Nước & Ánh sáng, Đèn LED ban đêm, Tổng hợp báo cáo

---

## SLIDE 1: TRANG BÌA (TITLE SLIDE)
- **Tiêu đề chính:** HỆ THỐNG GẬY THÔNG MINH IOT HỖ TRỢ NGƯỜI KHIẾM THỊ (SMART CANE IOT)
- **Học phần:** Thực tập & Bài tập lớn IoT — Học viện Công nghệ Bưu chính Viễn thông (PTIT)
- **Giảng viên hướng dẫn:** [Tên Thầy/Cô]
- **Nhóm thực hiện:** Nhóm 4 thành viên (Giang, Tùng, Duy, Đạt)
- **Thời gian:** Học kỳ I — Năm học 2026

---

## SLIDE 2: ĐẶT VẤN ĐỀ & MỤC TIÊU DỰ ÁN
- **Thực trạng:**
  - Người khiếm thị gặp vô vàn nguy hiểm khi di chuyển ngoài đường: va chạm vật cản ở tầm thấp/cao, trượt ngã vào vũng nước sâu/hố ga ngập, lạc đường hoặc gặp sự cố bất ngờ mà người thân không hay biết.
  - Các loại gậy truyền thống chỉ phát hiện vật cản bằng cách va chạm trực tiếp cơ học, không có khả năng cảnh báo sớm từ xa hay định vị.
- **Mục tiêu giải pháp:**
  - Chế tạo nguyên mẫu gậy thông minh tích hợp đa cảm biến (Khoảng cách, Nước ngập, Ánh sáng, Góc nghiêng, Định vị GPS).
  - Phản hồi rung/chuông tại chỗ cho người dùng và truyền dữ liệu thời gian thực lên Web Dashboard cho gia đình/người giám sát.

---

## SLIDE 3: KIẾN TRÚC TỔNG THỂ & LUỒNG DỮ LIỆU (SYSTEM ARCHITECTURE)
- **Tầng Thiết bị (Perception Layer):** ESP32 DevKit V1 kết nối HC-SR04, Water Sensor, LDR, MPU6050, GPS NEO-6M, Còi, Mô tơ, LED.
- **Tầng Mạng & Giao vận (Transport Layer):** Kết nối Wi-Fi băng tần 2.4GHz &rarr; Giao thức MQTT (Mosquitto Broker) đảm bảo độ trễ thấp (<200ms) và tiết kiệm năng lượng.
- **Tầng Xử lý & Lưu trữ (Application / Backend Layer):**
  - Backend Spring Boot: Tiếp nhận bản tin MQTT, xử lý chống trùng lặp sự kiện (`event_id`), lưu lịch sử vào PostgreSQL, kích hoạt thông báo Telegram.
- **Tầng Giám sát (Presentation Layer):**
  - Web Dashboard thời gian thực độc lập: Leaflet Map, Hàng rào địa lý Geofence, cảnh báo âm thanh Web Audio API.

---

## SLIDE 4: SƠ ĐỒ ĐẤU NỐI & PHÂN BỔ CHÂN PHẦN CỨNG (PINOUT)
- *Phân công:* Thiết kế bởi Giang & Cả nhóm thống nhất tại `docs/PINOUT.md`.
- **Nguyên tắc kỹ thuật sống còn:**
  - Khối cảm biến Analog (Water Sensor & LDR) bắt buộc dùng **ADC1 (GPIO 34 & 35)** để không bị tê liệt khi bật Wi-Fi.
  - Tách biệt nguồn 5V (Echo siêu âm, Mô tơ qua transistor) và 3.3V an toàn cho ESP32.
  - Sử dụng cầu phân áp bảo vệ chân ESP32 tránh quá áp 3.3V.

---

## SLIDE 5: MODULE PHẦN CỨNG 1 — VẬT CẢN & CẢNH BÁO RUNG (TÙNG)
- **Cảm biến siêu âm HC-SR04:** Đo cự ly vật cản từ 2cm đến 400cm.
- **Thuật toán phản hồi xúc giác 3 mức độ:**
  - Khoảng cách > 150cm: An toàn &rarr; Mô tơ TẮT.
  - Khoảng cách 50 – 150cm: Cảnh báo &rarr; Rung ngắt quãng nhịp vừa.
  - Khoảng cách < 50cm: Nguy hiểm &rarr; Rung liên tục mức tối đa.
- **Module GPS NEO-6M:** Cập nhật kinh độ/vĩ độ định kỳ gửi về hệ thống.

---

## SLIDE 6: MODULE PHẦN CỨNG 2 — AN TOÀN SOS & GẬY ĐỔ (DUY)
- **Cảm biến gia tốc MPU6050:**
  - Phân tích góc nghiêng 3 trục để nhận diện tình huống **Gậy bị rơi / ngã**.
  - Cơ chế đếm lùi 10 giây trước khi phát báo động (cho phép hủy bỏ nếu người dùng nhặt gậy lên bình thường).
- **Nút bấm khẩn cấp SOS:**
  - Thuật toán chống dội phím (Debounce).
  - Yêu cầu nhấn giữ 3 giây để tránh chạm nhầm &rarr; Kích hoạt chuông còi tại chỗ và gửi bản tin khẩn cấp `EVENT_SOS` lên Server/Telegram.

---

## SLIDE 7: MODULE PHẦN CỨNG 3 — CẢM BIẾN NƯỚC & ĐÈN BAN ĐÊM (ĐẠT)
- **Cảm biến tiếp xúc nước (Water Sensor - GPIO 35):**
  - Hoạt động dựa trên sự thay đổi điện trở khi các bản cực đồng tiếp xúc nước.
  - Thuật toán lọc nhiễu trung bình trượt 5 mẫu (Moving Average).
  - Ngưỡng kích hoạt: ADC $\ge$ 1500 &rarr; Kích hoạt rung cảnh báo vũng nước sâu hoặc ngập úng.
- **Quang trở LDR (GPIO 34) & Đèn LED ban đêm (GPIO 32):**
  - Tự động bật đèn trợ sáng khi trời tối, nâng cao khả năng nhận diện cho các phương tiện giao thông.
  - **Thuật toán trễ Hysteresis:**
    - Bật đèn khi ADC < 1600 (Trời tối sẩm).
    - Tắt đèn khi ADC > 2100 (Trời sáng rõ).
    - Vùng đệm 1600 – 2100 giữ nguyên trạng thái &rarr; Triệt tiêu hoàn toàn hiện tượng đèn chớp nháy (Flickering).

---

## SLIDE 8: GIAO DIỆN GIÁM SÁT WEB DASHBOARD (ĐẠT)
- **Công nghệ xây dựng:** HTML5, CSS3 Glassmorphism chuẩn cao cấp, JavaScript ES6.
- **Các tính năng nổi bật:**
  - **Bản đồ Leaflet thời gian thực:** Định vị GPS của gậy với hiệu ứng radar pulsing và đường vết di chuyển (Breadcrumb).
  - **Hàng rào địa lý Geofence (350m quanh PTIT):** Tự động phát hiện và đổi màu Đỏ khi người khiếm thị đi ra ngoài phạm vi an toàn.
  - **Hệ thống cảnh báo âm thanh:** Tích hợp bộ tổng hợp âm thanh Web Audio API (không cần file âm thanh ngoài).
  - **Hỗ trợ chế độ kép (Dual-Mode):** Tự động chuyển đổi mượt mà giữa Chế độ Giả lập (Mock) và Kết nối Backend thật (WebSocket/REST).

---

## SLIDE 9: BỘ GIẢ LẬP 8 KỊCH BẢN & CHUẨN DỮ LIỆU MQTT CONTRACT (ĐẠT)
- Cho phép demo trực quan 8 tình huống thực tế trước hội đồng chấm thi:
  1. *Trạng thái bình thường.*
  2. *Vật cản rất gần (<50cm).*
  3. *Chạm vũng nước sâu (ADC 2650).*
  4. *Trời tối sẩm &rarr; Tự động bật LED an toàn.*
  5. *Báo động SOS khẩn cấp.*
  6. *Nghi ngờ gậy bị ngã (MPU6050).*
  7. *Ra khỏi vùng an toàn Geofence.*
  8. *Mất kết nối thiết bị (Last Will and Testament).*
- Khung xuất chuỗi JSON trực tiếp thời gian thực tuân thủ tuyệt đối chuẩn `docs/MQTT_CONTRACT.md`.

---

## SLIDE 10: KẾT QUẢ THỰC NGHIỆM & ĐỐI CHIẾU MÔ PHỎNG VS MẠCH THẬT
- **Mô phỏng Wokwi:**
  - 100% logic module môi trường và trễ Hysteresis được kiểm thử trên Wokwi thành công.
- **Đo kiểm mạch thật trên ESP32:**
  - Đo đạc chính xác sai số ADC giữa không khí khô (185 – 350 ADC) và nước máy (2280 – 2650 ADC).
  - Đèn LED phản hồi chuẩn xác theo quang trở LDR mà không bị nhiễu điện áp.
- **Hiệu năng hệ thống:**
  - Chu kỳ lấy mẫu cảm biến: 100ms non-blocking, không gây trễ vòng lặp chính.
  - Giao diện Web phản hồi mượt mà ở tần số 60 FPS.

---

## SLIDE 11: ĐÁNH GIÁ & TIÊU CHÍ NGHIỆM THU
- [x] Hoàn thiện 4 bộ mô phỏng Wokwi theo module độc lập.
- [x] Chuẩn hóa hợp đồng dữ liệu JSON MQTT dùng chung cho toàn bộ nhóm.
- [x] Giao diện Web Dashboard độc lập, responsive trên cả máy tính và điện thoại.
- [x] Đóng gói các module firmware C++ tách biệt (`EnvironmentModule`, `ObstacleModule`, `SafetyModule`).
- [x] Báo cáo kỹ thuật chi tiết, video minh chứng và bảng đo kiểm thực nghiệm.

---

## SLIDE 12: KẾT LUẬN & HƯỚNG PHÁT TRIỂN TIẾP THEO
- **Kết luận:**
  - Đề tài đã hoàn thành xuất sắc các mục tiêu cốt lõi của nguyên mẫu Gậy Thông Minh IoT (Phát hiện vật cản, Chạm nước, Chiếu sáng ban đêm, SOS, Định vị và Giám sát từ xa).
  - Các module hoạt động ổn định, phân tách rõ ràng và dễ dàng mở rộng.
- **Hướng phát triển:**
  - Tích hợp thêm cảm biến siêu âm hướng xuống góc 45° để cảnh báo hụt chân/bậc cầu thang.
  - Ứng dụng trí tuệ nhân tạo (TinyML) trên ESP32 để nhận diện người ngã chính xác hơn.
- **HỎI & ĐÁP (Q&A):** Xin chân thành cảm ơn Quý Thầy/Cô và các bạn đã lắng nghe!
