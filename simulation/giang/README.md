# Giang — G05: ESP32 Wokwi → MQTT

Đây là **mô phỏng riêng** và chỉ gửi **dữ liệu giả**. Không nạp firmware này lên gậy thật hoặc xem GPS ở đây là dữ liệu thực.

1. Tạo project ESP32 tại https://wokwi.com/projects/new/esp32.
2. Dán `sketch.ino` và `diagram.json`, tạo `libraries.txt` với nội dung trong thư mục.
3. Bấm Start Simulation; Serial xuất `[MQTT] connected` và `[PUBLISH] OK` khi broker phản hồi.
4. Trong MQTTX, host `test.mosquitto.org`, port `1883`, subscribe `smartcane/cane-giang-demo/telemetry`.
5. Kiểm tra JSON đều mỗi 5 giây, `distance_cm` thay đổi, các trường khác có giá trị mock.

**Cảnh báo:** broker công khai không bảo mật. Chỉ dùng số liệu giả, không gửi GPS thực, token hay thông tin cá nhân. MQTT host có thể không khả dụng; nếu lỗi, thử lại/đổi broker được cấp phép và ghi rõ môi trường. Wokwi không trỏ tới Docker localhost của máy cá nhân. Trình mô phỏng không có NTP chuẩn trong ví dụ nên không gửi timestamp giả; backend lưu `receivedAt`.

**Minh chứng phải nộp:** link project Wokwi, ảnh Serial, ảnh MQTTX, mô tả test thành công/thất bại. Thêm file minh chứng vào PR của bạn sau khi tự chạy.
