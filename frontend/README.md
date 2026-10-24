# Dashboard — Đạt

Hiện có `index.html` và `mock.js` hiển thị dữ liệu **giả**; Leaflet tải CSS/JS và map tile qua Internet. Chạy `python -m http.server 5500 -d frontend`, mở http://localhost:5500.

Công việc của Đạt: bổ sung hiển thị Water, LDR, LED, SOS, trạng thái kết nối, GPS/no-fix và tình trạng dữ liệu lỗi. Sau khi Giang cung cấp API, dùng adapter chuyển mock ↔ REST/WebSocket mà không đổi hợp đồng trường trong `docs/MQTT_CONTRACT.md`. Không hardcode token trên frontend.
