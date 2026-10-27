/*
 * Smart Cane IoT — PTIT | Module Môi Trường & Đèn (Đạt)
 * Mô phỏng Wokwi: Cảm biến nước (Water Sensor) + Cảm biến ánh sáng (LDR) + Đèn LED ban đêm
 * 
 * Sơ đồ chân (khớp docs/PINOUT.md):
 * - GPIO 35 (ADC1_CH7): Tín hiệu Analog cảm biến nước (Water Sensor)
 * - GPIO 34 (ADC1_CH6): Tín hiệu Analog cảm biến ánh sáng (LDR)
 * - GPIO 32: Điều khiển LED ban đêm (qua trở hạn dòng 220 Ohm)
 */

#include <Arduino.h>

// Định nghĩa chân phần cứng
const int PIN_WATER_ADC = 35;
const int PIN_LDR_ADC   = 34;
const int PIN_NIGHT_LED = 32;

// Cấu hình ngưỡng cảm biến
// ESP32 ADC: 12-bit (0 - 4095), điện áp 0 - 3.3V
const int WATER_THRESHOLD = 1500;       // ADC > 1500: Phát hiện có nước / ẩm ướt
const int LIGHT_ON_THRESHOLD  = 1600;   // ADC < 1600: Trời tối -> Bật đèn
const int LIGHT_OFF_THRESHOLD = 2100;   // ADC > 2100: Trời đủ sáng -> Tắt đèn (Trễ hysteresis chống nhấp nháy)

// Thời gian chu kỳ gửi dữ liệu
const unsigned long TELEMETRY_INTERVAL_MS = 2000;
const unsigned long SENSOR_SAMPLE_INTERVAL_MS = 100;

// Biến trạng thái
unsigned long lastTelemetryTime = 0;
unsigned long lastSampleTime = 0;

int rawWaterAdc = 0;
int rawLdrAdc = 0;
bool isWaterDetected = false;
bool isLedActive = false;

// Bộ lọc làm mịn dữ liệu ADC (Moving average đơn giản)
long waterAdcAccumulator = 0;
long ldrAdcAccumulator = 0;
int sampleCount = 0;

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(PIN_WATER_ADC, INPUT);
  pinMode(PIN_LDR_ADC, INPUT);
  pinMode(PIN_NIGHT_LED, OUTPUT);
  digitalWrite(PIN_NIGHT_LED, LOW);

  Serial.println(F("=================================================="));
  Serial.println(F("  SMART CANE IoT - ENVIRONMENT MODULE (DAT)       "));
  Serial.println(F("  PTIT 2026 | Wokwi Simulation Started            "));
  Serial.println(F("=================================================="));
  Serial.println(F("Pins: WATER_ADC=35 | LDR_ADC=34 | NIGHT_LED=32"));
  Serial.println(F("Mode: Auto Light Detection (Hysteresis) + Water Alarm"));
  Serial.println(F("--------------------------------------------------"));
}

void processSensors() {
  // Đọc giá trị tức thời từ ADC
  int curWater = analogRead(PIN_WATER_ADC);
  int curLdr   = analogRead(PIN_LDR_ADC);

  waterAdcAccumulator += curWater;
  ldrAdcAccumulator += curLdr;
  sampleCount++;

  if (sampleCount >= 5) {
    rawWaterAdc = waterAdcAccumulator / sampleCount;
    rawLdrAdc   = ldrAdcAccumulator / sampleCount;
    
    waterAdcAccumulator = 0;
    ldrAdcAccumulator = 0;
    sampleCount = 0;

    // 1. Xử lý cảm biến nước
    isWaterDetected = (rawWaterAdc >= WATER_THRESHOLD);

    // 2. Xử lý cảm biến ánh sáng & LED tự động (Hysteresis)
    if (!isLedActive && rawLdrAdc < LIGHT_ON_THRESHOLD) {
      isLedActive = true;
      digitalWrite(PIN_NIGHT_LED, HIGH);
    } else if (isLedActive && rawLdrAdc > LIGHT_OFF_THRESHOLD) {
      isLedActive = false;
      digitalWrite(PIN_NIGHT_LED, LOW);
    }
  }
}

void printTelemetryJson() {
  // Tạo định dạng JSON đúng theo contract docs/MQTT_CONTRACT.md
  Serial.print(F("{\"device_id\":\"cane-01\","));
  Serial.print(F("\"water_detected\":"));
  Serial.print(isWaterDetected ? "true" : "false");
  Serial.print(F(",\"water_adc\":"));
  Serial.print(rawWaterAdc);
  Serial.print(F(",\"light_level\":"));
  Serial.print(rawLdrAdc);
  Serial.print(F(",\"led_on\":"));
  Serial.print(isLedActive ? "true" : "false");
  Serial.print(F(",\"status\":\""));
  if (isWaterDetected) {
    Serial.print(F("WATER_WARNING"));
  } else if (isLedActive) {
    Serial.print(F("NIGHT_MODE"));
  } else {
    Serial.print(F("NORMAL"));
  }
  Serial.println(F("\"}"));
}

void printStatusDashboard() {
  Serial.println(F("\n--- [BẢNG THEO DÕI CẢM BIẾN MÔI TRƯỜNG] ---"));
  Serial.print(F("Water Sensor ADC (GPIO 35): "));
  Serial.print(rawWaterAdc);
  Serial.print(F(" / 4095 -> "));
  if (isWaterDetected) {
    Serial.println(F("[!] CẢNH BÁO: PHÁT HIỆN CHẠM NƯỚC"));
  } else {
    Serial.println(F("[OK] Khô ráo"));
  }

  Serial.print(F("LDR Sensor ADC (GPIO 34):   "));
  Serial.print(rawLdrAdc);
  Serial.print(F(" / 4095 -> "));
  if (rawLdrAdc < LIGHT_ON_THRESHOLD) {
    Serial.println(F("Môi trường TỐI"));
  } else if (rawLdrAdc > LIGHT_OFF_THRESHOLD) {
    Serial.println(F("Môi trường SÁNG"));
  } else {
    Serial.println(F("Vùng trễ chuyển tiếp"));
  }

  Serial.print(F("Đèn LED cảnh báo (GPIO 32):  "));
  Serial.println(isLedActive ? F("[ON] ĐANG BẬT") : F("[OFF] ĐANG TẮT"));
  Serial.println(F("---------------------------------------------"));
}

void loop() {
  unsigned long now = millis();

  // Đọc mẫu cảm biến mỗi 100ms
  if (now - lastSampleTime >= SENSOR_SAMPLE_INTERVAL_MS) {
    lastSampleTime = now;
    processSensors();
  }

  // Xuất telemetry mỗi 2 giây
  if (now - lastTelemetryTime >= TELEMETRY_INTERVAL_MS) {
    lastTelemetryTime = now;
    printTelemetryJson();
    // printStatusDashboard(); // Bật nếu muốn xem log chi tiết
  }
}
