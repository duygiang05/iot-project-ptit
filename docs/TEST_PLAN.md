# TEST PLAN — MINIMUM ACCEPTANCE

| ID | Kiểm thử | Đạt khi |
|---|---|---|
| T01 | HC-SR04 vật cản | Khoảng cách thay đổi đúng chiều; timeout là null; rung/LED phản ánh trạng thái |
| T02 | GPS | Có fix thì có tọa độ hợp lệ; mất fix thì `gps_valid=false` và tọa độ null |
| D01 | SOS nút | Chống dội, giữ đủ thời gian theo thiết kế đã duyệt, phát một event duy nhất |
| D02 | MPU6050 | Đánh dấu nghi ngờ gậy đổ, không tuyên bố phát hiện ngã người |
| Đ01 | LDR/LED | Ánh sáng thay đổi → LED đổi trạng thái hợp lý |
| Đ02 | Water | Có chạm nước → đổi trạng thái, đo hiệu chuẩn phần cứng |
| G01 | MQTT | Publish/subscribe JSON theo đúng topic, reconnect sau ngắt |
| G02 | Backend | Dữ liệu được parse; JSON lỗi bị từ chối/log phù hợp; event_id chống gửi lặp |
| G03 | Web | Hiển thị mock trước; sau chuyển API; mất GPS không hiển thị điểm giả như dữ liệu thật |
| G04 | Telegram | Chỉ backend gửi khi sự kiện đủ điều kiện, tránh spam |
| SYS | Offline/sụt áp | Wi-Fi mất vẫn rung/còi; test nguồn/motor không reset bất thường |

Mỗi PR phải ghi: **đã test bằng gì, bước tái hiện, expected/actual, ảnh/log/link Wokwi**. Ngưỡng cụ thể ở thử nghiệm do owner đề xuất và Giang/nhóm duyệt ảnh hưởng hệ thống.
