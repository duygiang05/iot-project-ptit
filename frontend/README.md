# Frontend Dashboard — Smart Cane IoT (PTIT)

**Owner phụ trách:** Đạt  
**Mục tiêu bàn giao:** Đ01 (Thiết kế khung Web), Đ02 (Bản đồ Leaflet & Dữ liệu giả lập), Đ05 (Hoàn thiện Web demo).

---

## 1. Tính năng nổi bật của Dashboard
- **Thiết kế Dark Glassmorphism chuẩn cao cấp:** Giao diện tối hiện đại, tương thích hoàn hảo trên cả máy tính để bàn, laptop và thiết bị di động (Responsive).
- **Bản đồ trực tiếp (Leaflet Map):**
  - Hiển thị vị trí thực tế của gậy thông minh với icon gậy phát xung radar (Pulsing).
  - Vòng tròn hàng rào an toàn địa lý (**Geofence** - bán kính 350m xung quanh PTIT Hà Đông) tự động đổi màu Đỏ khi ra ngoài vùng.
  - Vết di chuyển thời gian thực (Breadcrumb trail).
- **Bộ thẻ số liệu thời gian thực (Telemetry Cards):**
  - 📏 **Khoảng cách vật cản (HC-SR04):** Đo cự ly cm, thanh đo chuyển màu An toàn / Cảnh báo / Nguy hiểm, báo cấp độ rung của mô tơ.
  - 💧 **Cảm biến nước (Water Sensor - Đạt phụ trách):** Phát hiện tiếp xúc nước, thanh đo ADC / 4095, cảnh báo vũng nước sâu.
  - 💡 **Môi trường & Đèn (LDR & LED - Đạt phụ trách):** Mức sáng ADC, trạng thái bật/tắt đèn ban đêm tự động với thuật toán trễ Hysteresis.
  - 🧭 **Trạng thái an toàn & SOS:** Theo dõi góc nghiêng gậy ngã (MPU6050) và nút bấm khẩn cấp SOS.
  - 🔋 **Dung lượng Pin 18650:** Tỷ lệ % và điện áp ước tính.
- **Biểu ngữ cảnh báo khẩn cấp (Emergency Banner):** Tự động bung ra kèm hiệu ứng nhấp nháy và âm thanh cảnh báo âm lượng vừa phải (tổng hợp qua Web Audio API, không cần file MP3 phụ).
- **Nhật ký sự kiện thời gian thực (Event Log):** Lọc theo Khẩn cấp, Nước, Môi trường và cho phép xuất file `.json`.
- **Bộ giả lập kịch bản (Scenario Simulator):** 
  - 8 nút chọn nhanh 8 tình huống kiểm thử trong đồ án: *Bình thường, Vật cản gần, Chạm vũng nước, Trời tối bật đèn, Báo động SOS, Gậy nghi ngờ đổ, Ra ngoài Geofence, Mất kết nối.*
  - Thanh trượt điều chỉnh cự ly, ADC nước, ADC ánh sáng thủ công để quan sát giao diện phản hồi tức thì.
  - Hộp hiển thị chuỗi JSON Telemetry theo đúng chuẩn hợp đồng `docs/MQTT_CONTRACT.md`.

---

## 2. Hướng dẫn chạy thử nghiệm Web
Từ thư mục gốc của dự án, khởi chạy server HTTP cục bộ bằng Python:

```powershell
python -m http.server 5500 -d frontend
```

Sau đó mở trình duyệt truy cập:
👉 **`http://localhost:5500`** (hoặc `http://127.0.0.1:5500`)

---

## 3. Cấu trúc mã nguồn Frontend
- `index.html`: Khung cấu trúc HTML5 ngữ nghĩa, tích hợp Leaflet và Google Fonts.
- `style.css`: Hệ thống thiết kế CSS Glassmorphism hoàn chỉnh, hỗ trợ animation mượt mà.
- `app.js`: Logic xử lý bản đồ Leaflet, tổng hợp âm thanh cảnh báo (Web Audio API), quản lý state, bộ giả lập 8 kịch bản và sinh chuỗi JSON contract.
