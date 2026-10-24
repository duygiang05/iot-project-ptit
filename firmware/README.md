# Firmware ESP32 — PlatformIO / Arduino

Cấu hình board `esp32dev`, Serial 115200. `src/main.cpp` ban đầu chỉ **mock Serial**, không có MQTT/điều khiển cảm biến. `include/PinConfig.h` là pinout dự thảo.

- Giang: quản lý `main.cpp`, MQTT service, chốt `PinConfig.h`.
- Tùng: `ObstacleModule.*`, `GpsModule.*`.
- Duy: `SafetyModule.*`.
- Đạt: `EnvironmentModule.*`.

**API chung:** xem `docs/MODULE_INTERFACE.md`. Wokwi từng người trong `simulation/<ten>/`, không thêm nhiều `setup()`/`loop()` vào thư mục `src/`.

**Build:** `pio run -d firmware` từ root (nếu CLI PlatformIO được cài); `pio device monitor -d firmware -b 115200` với board kết nối. Nhớ cấu hình đúng cổng serial.
