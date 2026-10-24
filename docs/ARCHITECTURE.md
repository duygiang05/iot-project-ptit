# ARCHITECTURE — CHỐT CHUNG

```text
[HC-SR04 | GPS | MPU6050 | SOS | Water | LDR | motor/LED/buzzer]
                     |
            ESP32 (Arduino C++)
                     | Wi-Fi, MQTT JSON
                     v
             Mosquitto Broker
                     |
            Spring Boot subscriber
             /       |         \
      PostgreSQL   REST/WS     Telegram (EVENT, backend duy nhất gửi)
                    |
             Web Leaflet
```

**Tính độc lập:** cảnh báo gần người dùng (rung/còi) do ESP32 xử lý trực tiếp, không phụ thuộc Internet. MQTT và Web/Telegram phụ thuộc Wi-Fi/broker/backend. GPS mất tín hiệu: `gps_valid=false`, tọa độ `null`.

**Ownership:** Giang sở hữu phần tích hợp, MQTT/backend/infra. Tùng sở hữu vật cản, motor rung, GPS. Duy sở hữu SOS, MPU6050, buzzer. Đạt sở hữu Water/LDR/LED và Web.

**Ngoài phạm vi mặc định:** phát hiện hố đáng tin cậy, chẩn đoán ngã người, nhận diện vật bằng AI, điều hướng tự động. HC-SR04 hướng xuống chỉ là tính năng nghiên cứu tùy chọn.

**Nguyên tắc:** Chốt *hợp đồng dữ liệu* trước; ngưỡng thuật toán/thời gian lọc/UX do owner nghiên cứu, đo kiểm, ghi nhận và gửi PR. Thay đổi giao thức/GPIO phải được duyệt chung.
