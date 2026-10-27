#include <Arduino.h>

// Định nghĩa chân Cảm biến 1: Phía trước (Vật cản)
const int TRIG_FRONT = 5;
const int ECHO_FRONT = 18;

// Định nghĩa chân Cảm biến 2: Hướng xuống (Phát hiện hụt chân / bậc thềm)
const int TRIG_DOWN = 4;
const int ECHO_DOWN = 19;

// Chân điều khiển mô-tơ rung qua Transistor/MOSFET (PWM)
const int VIB_PIN = 21;

// Cấu hình PWM cho ESP32 (Arduino Core v3.x)
const int PWM_FREQ = 1000;    // Tần số PWM 1kHz
const int PWM_RESOLUTION = 8; // 8-bit (0 - 255)

// Biến quản lý thời gian cho rung ngắt quãng
unsigned long previousMillis = 0;
bool vibState = false;

// Hàm đo khoảng cách tổng quát cho từng cảm biến
float measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Đọc thời gian phản hồi (timeout 30ms ~ khoảng cách tối đa 5m)
  long duration = pulseIn(echoPin, HIGH, 30000); 
  if (duration == 0) {
    return -1; // Lỗi hoặc vượt quá tầm đo
  }
  return duration * 0.0343 / 2.0; // Trả về khoảng cách theo cm
}

void setup() {
  Serial.begin(115200);

  // Cấu hình chân I/O
  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);
  pinMode(TRIG_DOWN, OUTPUT);
  pinMode(ECHO_DOWN, INPUT);
  pinMode(VIB_PIN, OUTPUT);

  // Khởi tạo kênh PWM trên ESP32
  ledcAttach(VIB_PIN, PWM_FREQ, PWM_RESOLUTION);

  Serial.println("Hệ thống Gậy IoT 2 cảm biến HC-SR04 đã khởi động.");
}

void loop() {
  // Đo khoảng cách từ 2 cảm biến
  float distFront = measureDistance(TRIG_FRONT, ECHO_FRONT);
  float distDown = measureDistance(TRIG_DOWN, ECHO_DOWN);

  // In log debug ra Serial Monitor
  Serial.print("Truoc: ");
  Serial.print(distFront);
  Serial.print(" cm | Huong xuong: ");
  Serial.print(distDown);
  Serial.println(" cm");

  unsigned long currentMillis = millis();
  int pwmVal = 0;

  // --- LOGIC ƯU TIÊN CẢNH BÁO ---
  
  // 1. ƯU TIÊN CAO NHẤT: Phát hiện hụt chân / bậc thềm đi xuống (Cảm biến hướng xuống)
  // Giả sử khoảng cách bình thường từ gậy xuống mặt đất là 20-40cm. 
  // Nếu khoảng cách vượt quá 50cm (hụt hố, bậc thang đi xuống), kích hoạt cảnh báo nguy hiểm ngay.
  if (distDown > 50.0 && distDown > 0) {
    // Rung nhịp cực nhanh (tần suất 150ms) để cảnh báo nguy cơ té ngã
    if (currentMillis - previousMillis >= 150) {
      previousMillis = currentMillis;
      vibState = !vibState;
    }
    pwmVal = vibState ? 255 : 0;
  } 
  // 2. ƯU TIÊN THỨ HAI: Vật cản phía trước mặt
  else if (distFront >= 0 && distFront < 50.0) {
    // Ngưỡng gần (<50cm): Rung liên tục, cường độ mạnh
    pwmVal = 255;
  } 
  else if (distFront >= 50.0 && distFront <= 150.0) {
    // Ngưỡng trung bình (50 - 150cm): Rung ngắt quãng nhịp vừa (300ms)
    if (currentMillis - previousMillis >= 300) {
      previousMillis = currentMillis;
      vibState = !vibState;
    }
    pwmVal = vibState ? 180 : 0;
  } 
  else {
    // Không có vật cản và mặt đất bình thường: Tắt rung
    pwmVal = 0;
    vibState = false;
  }

  // Xuất tín hiệu PWM ra mô-tơ rung
  ledcWrite(VIB_PIN, pwmVal);

  delay(100); // Độ trễ chu kỳ đọc cảm biến
}