# Smart Cane IoT — PTIT | Starter tinh gọn

**Bộ khung chung cho 4 thành viên, không phải sản phẩm đã hoàn thiện.** Các ngưỡng cảm biến và thuật toán được chủ module thử nghiệm rồi đề xuất; chỉ khóa cách giao tiếp và quy trình tích hợp.

## Bắt đầu hôm nay
1. Cả nhóm đọc `docs/ARCHITECTURE.md`, `docs/TEAM_RESPONSIBILITIES.md`, `docs/MQTT_CONTRACT.md`, `docs/MODULE_INTERFACE.md`, `docs/PINOUT.md`.
2. Giang xác nhận **pinout dự thảo** cùng nhóm trước khi đấu mạch. Mỗi người clone repo, tạo feature branch (xem `docs/GIT_WORKFLOW.md`).
3. Tùng, Duy, Đạt lập trình Wokwi riêng trong `simulation/<ten>/`; Giang mô phỏng Wi-Fi/MQTT và phát triển backend/hạ tầng. Mỗi người nộp sketch `.ino`, `diagram.json`, README/link và bằng chứng chạy thử.
4. Chuyển logic đã thử vào `firmware/include/` + `firmware/src/` dưới dạng module; chỉ Giang quản lý điểm tích hợp `main.cpp` và `PinConfig.h`.
5. PR có kết quả build/test, log/ảnh và giải thích thay đổi; không sửa contract chung tùy ý.

## Cấu trúc và trách nhiệm
| Thư mục | Vai trò | Owner |
|---|---|---|
| `docs/` | Kiến trúc, pinout, MQTT, module interface, Git, tích hợp, kiểm thử | Chung; Giang chốt contract |
| `simulation/giang/` | Wokwi Wi-Fi/MQTT + JSON mock | Giang |
| `simulation/tung/` | Wokwi HC-SR04 + cảnh báo mô phỏng | Tùng |
| `simulation/duy/` | Wokwi SOS, buzzer, trạng thái gậy đổ | Duy |
| `simulation/dat/` | Wokwi LDR/LED + tín hiệu nước giả lập | Đạt |
| `firmware/` | ESP32 PlatformIO/C++: code chung và các module | Cả nhóm, theo sở hữu file |
| `backend/` | Spring Boot/MQTT/REST/DB (hiện mới README) | Giang |
| `infra/` | Docker Compose Mosquitto + PostgreSQL | Giang |
| `frontend/` | Dashboard Leaflet mock (HTML/JS) | Đạt |
| `tests/` | JSON mẫu và tiêu chí kiểm thử | Cả nhóm |

**Phân công file ESP32:** Giang `main.cpp`, `PinConfig.h`, `MqttService.*`; Tùng `ObstacleModule.*`, `GpsModule.*`; Duy `SafetyModule.*`; Đạt `EnvironmentModule.*`. Các file module chưa có sẵn và sẽ được tạo theo tiến độ.

## Chạy nhanh
- Hạ tầng: sao chép `.env.example` thành `.env` ở thư mục gốc, đặt `POSTGRES_PASSWORD` an toàn; `docker compose --env-file .env -f infra/compose.yaml up -d`.
- Web mock: `python -m http.server 5500 -d frontend` rồi truy cập `http://localhost:5500`.
- ESP32: mở `firmware/` bằng VS Code + PlatformIO. Hiện `main.cpp` **chỉ in JSON mock lên Serial**, chưa kết nối Wi-Fi/MQTT và chưa đọc cảm biến.
- Dữ liệu mẫu: `tests/telemetry.sample.json`, `tests/event.sample.json`.
- Wokwi browser **không kết nối trực tiếp** được tới Mosquitto ở `localhost`; hạ tầng localhost dành cho thử backend/CLI. Đừng mở broker anonymous ra Internet.

## Chốt chung và quyền tối ưu
- **Cố định chung:** kiến trúc, tech stack, quyền sở hữu file, chủ đề MQTT, JSON key/type/unit, khung module, quy tắc Git, yêu cầu an toàn và tiêu chí test.
- **Owner tự tối ưu:** Tùng (ngưỡng vật cản, mẫu rung, GPS); Duy (thuật toán MPU6050, chống dội nút); Đạt (ngưỡng nước/ánh sáng, Web); Giang (chi tiết backend, DB, MQTT retry).
- **Phải duyệt chung trước đổi:** GPIO, JSON schema/topic, giao diện module, hành vi an toàn/SOS, mức điện áp và cấp nguồn.

## Lưu ý an toàn
Nguyên mẫu học tập, không thay gậy dò đường được chứng nhận. Water Sensor chỉ phát hiện nước khi tiếp xúc; MPU6050 chỉ hỗ trợ nhận diện **gậy** đổ, không khẳng định người ngã. Echo HC-SR04 5V phải chuyển mức an toàn trước ESP32; motor dùng driver/transistor + diode. Thử USB trước pin, không vừa sạc TP4056 thông thường vừa vận hành nếu không có power-path. Không commit mật khẩu Wi-Fi, Telegram token hoặc tọa độ cá nhân.
