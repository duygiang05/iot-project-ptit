# G02 — Điều phối và tiêu chí nghiệm thu

## Ma trận bàn giao
| Người | Mô phỏng độc lập | Code chính thức sau mô phỏng | Minh chứng |
|---|---|---|---|
| Giang | Wi-Fi, MQTT, JSON mock | MqttService, backend và infra | Wokwi + MQTTX + backend logs |
| Tùng | HC-SR04 + LED thay rung | ObstacleModule + GpsModule | khoảng cách, 3 mức và GPS test |
| Duy | SOS + buzzer + MPU6050 | SafetyModule | nhấn giữ/hủy, các ca gậy đổ |
| Đạt | LDR/LED + water mock | EnvironmentModule + frontend | giá trị môi trường và dashboard |

## Mỗi PR phải có
1. Mục tiêu và task ID; 2. Files thay đổi; 3. Lệnh chạy; 4. Kết quả thực chạy; 5. Ảnh/serial log; 6. Giới hạn/chưa làm.

## Chặn nghiệm thu
- Không tự ý sửa PinConfig, MQTT_CONTRACT hay main.cpp nếu không được Giang duyệt.
- Không tính 'Done' nếu chưa có test chạy thực tế.
- Không đưa mật khẩu, token, thông tin vị trí thật vào repo công khai.
- Không ghép breadboard/dây jumper lỏng để demo di chuyển.

## Thứ tự thực hiện Giang
G01/G02 khóa chuẩn → G05 Wokwi MQTT độc lập và G03 backend nhận MQTT (có thể song song) → G04 persist PostgreSQL + kiểm tra HTTP → kiểm thử tích hợp. G06 trở đi chỉ bắt đầu khi có phần cứng và module từ các thành viên.
