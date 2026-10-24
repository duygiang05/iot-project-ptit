# KẾ HOẠCH TÍCH HỢP

1. **Ngày khởi động**: cả nhóm clone repo, tạo nhánh, kiểm tra mô phỏng; Giang test infra và schema.
2. **Chạy riêng**: Tùng đo khoảng cách/LED giả; Duy SOS/buzzer; Đạt LDR/LED/Water + Web mock; Giang publish JSON giả từ Wokwi qua broker truy cập được.
3. **Ghép không phần cứng**: Backend subscribe JSON mẫu, ghi PostgreSQL, expose API; Web chuyển từ mock sang API; backend gửi Telegram thử từ sự kiện giả (không spam).
4. **Ghép firmware**: đưa từng module vào C++ và gọi từ `main.cpp`, test serial trước MQTT.
5. **Mạch thật**: nguồn USB trước, kiểm GPIO/điện áp/driver motor; sau đó mới pin 18650, kiểm dòng và độ ổn định.
6. **Demo**: vật cản → rung; nút SOS → event → backend → Telegram; trạng thái/định vị → Web; mất Wi-Fi/GPS không làm mất cảnh báo tại chỗ.

**Điều kiện merge:** build thành công; test chức năng đạt; không lộ secret; có README/test và ảnh/log; JSON tương thích mẫu; không phá quyền sở hữu file.

**Bất định cần kiểm thử:** Wokwi browser không gọi được `localhost` Mosquitto; broker localhost trong Compose chỉ phù hợp local. Không mở anonymous broker ra Internet. Nên test MQTT qua MQTTX/CLI local hoặc broker thử nghiệm kiểm soát được trước khi kết nối Wokwi.
