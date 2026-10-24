# PHÂN CÔNG VÀ PHẠM VI CHỈNH SỬA

| Người | Sở hữu chính | Wokwi | Đầu ra cần bàn giao |
|---|---|---|---|
| Giang | `infra/`, `backend/`, `firmware/src/main.cpp`, MQTT service và contract chung | `simulation/giang/` Wi-Fi/MQTT + JSON giả | Broker nhận JSON, backend subscribe, API skeleton |
| Tùng | `ObstacleModule.*`, `GpsModule.*` | `simulation/tung/` HC-SR04 + LED mô phỏng rung | Khoảng cách cm, tình trạng vật cản, GPS valid/lat/lon |
| Duy | `SafetyModule.*` | `simulation/duy/` SOS + buzzer + giả lập MPU6050 | Sự kiện SOS/CANCEL/CANE_FALL_SUSPECTED/ALERT |
| Đạt | `EnvironmentModule.*`, `frontend/` | `simulation/dat/` LDR/LED + mô phỏng Water | Water/LDR/LED data + dashboard mock |

**Không chia theo kiểu 4 bản `main.cpp`.** Mỗi người tạo `include/<Module>.h`, `src/<Module>.cpp`, test độc lập trong `simulation/<ten>/`. Giang duy nhất tổng hợp ở `main.cpp` và `PinConfig.h`. Nếu cần thay đổi pin, mở PR chỉnh `docs/PINOUT.md`, không tự sửa lén.

**Ngày đầu:** Giang thử MQTT/infra với JSON mẫu; Tùng HC-SR04; Duy SOS/buzzer; Đạt LDR/LED và Web mock. Mỗi người lưu code Wokwi vào `simulation/<ten>/`, push nhánh riêng kèm kết quả kiểm thử.
