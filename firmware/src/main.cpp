#include <Arduino.h>
#include <math.h>
#include <string.h>
#include <WiFi.h>
#include <Wire.h>
#include <ArduinoMqttClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#include "WifiSecrets.h" // local-only file; see WifiSecrets.example.h
#ifndef MQTT_USERNAME
#define MQTT_USERNAME ""
#endif
#ifndef MQTT_PASSWORD
#define MQTT_PASSWORD ""
#endif
#include "PinConfig.h"
#include "SafetyModule.h"
#include "TelemetryData.h"

static const char* DEVICE_ID = "cane-01";
static const char* TELEMETRY_TOPIC = "smartcane/cane-01/telemetry";
static const char* EVENT_TOPIC = "smartcane/cane-01/events";
static const char* STATUS_TOPIC = "smartcane/cane-01/status";
static constexpr uint32_t SEND_INTERVAL_MS = 5000;
static constexpr uint32_t RETRY_INTERVAL_MS = 5000;
static constexpr uint32_t MPU_SAMPLE_INTERVAL_MS = 50;

WiFiClient network;
MqttClient mqtt(network);
Adafruit_MPU6050 mpu;
SafetyModule safety;
bool mpuReady = false;
float uprightX = 0.0f, uprightY = 0.0f, uprightZ = 1.0f;
uint32_t lastWifiAttempt = 0, lastMqttAttempt = 0, lastSend = 0;
uint32_t lastMpuSample = 0, eventCounter = 0;
uint32_t seq = 0;
bool fallCountdownActive = false;
uint32_t fallCountdownStartedAt = 0, lastCountdownPrintAt = 0;

bool publishMessage(const char* topic, const char* payload, uint8_t qos, bool retained = false) {
  if (!mqtt.beginMessage(topic, retained, qos)) return false;
  mqtt.print(payload);
  // ArduinoMqttClient returns 1 on success and waits for PUBACK for QoS 1.
  return mqtt.endMessage() != 0;
}

// TODO from the sensor module owners: replace remaining telemetry placeholders.
TelemetryData readTelemetry() {
  TelemetryData data;
  data.distanceCm = 45.0f + static_cast<float>((seq++ % 10) * 10);
  return data;
}

bool calibrateUpright() {
  Serial.println("[MPU] Hold cane upright and still for 2 seconds.");
  float sx = 0, sy = 0, sz = 0;
  uint8_t samples = 0;
  const uint32_t started = millis();
  while (millis() - started < 2000) {
    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);
    const float x = accel.acceleration.x;
    const float y = accel.acceleration.y;
    const float z = accel.acceleration.z;
    const float magnitude = sqrtf(x*x + y*y + z*z);
    if (magnitude >= 7.35f && magnitude <= 12.26f) {
      sx += x / magnitude;
      sy += y / magnitude;
      sz += z / magnitude;
      ++samples;
    }
    delay(50);
  }
  if (samples < 10) return false;
  const float norm = sqrtf(sx*sx + sy*sy + sz*sz);
  if (norm < 0.1f) return false;
  uprightX = sx / norm;
  uprightY = sy / norm;
  uprightZ = sz / norm;
  return true;
}

void ensureWifi(uint32_t now) {
  if (WiFi.status() == WL_CONNECTED) return;
  if (lastWifiAttempt != 0 && now - lastWifiAttempt < RETRY_INTERVAL_MS) return;
  lastWifiAttempt = now;
  Serial.printf("[WIFI] connecting to %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void ensureMqtt(uint32_t now) {
  if (WiFi.status() != WL_CONNECTED || mqtt.connected()) return;
  if (lastMqttAttempt != 0 && now - lastMqttAttempt < RETRY_INTERVAL_MS) return;
  lastMqttAttempt = now;
  String clientId = String("smartcane-esp32-") + WiFi.macAddress();
  clientId.replace(":", "");
  Serial.printf("[MQTT] trying %s:%d\n", MQTT_HOST, MQTT_PORT);
  mqtt.setId(clientId.c_str());
  if (mqtt.connect(MQTT_HOST, MQTT_PORT)) {
    Serial.println("[MQTT] connected");
    publishMessage(STATUS_TOPIC, "online", 0, true);
  } else {
    Serial.printf("[MQTT] failed error=%d\n", mqtt.connectError());
  }
}

void publishTelemetry(uint32_t now) {
  if (!mqtt.connected() || now - lastSend < SEND_INTERVAL_MS) return;
  lastSend = now;
  TelemetryData d = readTelemetry();
  char json[320];
  snprintf(json, sizeof(json),
    "{\"device_id\":\"%s\",\"distance_cm\":%.1f,\"latitude\":null,"
    "\"longitude\":null,\"gps_valid\":false,\"water_detected\":%s,"
    "\"light_level\":%d,\"led_on\":%s,\"battery_voltage\":null}",
    DEVICE_ID, d.distanceCm, d.waterDetected ? "true" : "false",
    d.lightLevel, d.ledOn ? "true" : "false");
  const bool ok = publishMessage(TELEMETRY_TOPIC, json, 0);
  Serial.printf("[TELEMETRY] %s %s\n", ok ? "OK" : "FAILED", json);
}

const char* safetyEventName(SafetyEventType type) {
  switch (type) {
    case SafetyEventType::SOS: return "SOS";
    case SafetyEventType::CANE_FALL_SUSPECTED: return "CANE_FALL_SUSPECTED";
    case SafetyEventType::CANE_FALL_ALERT: return "CANE_FALL_ALERT";
    case SafetyEventType::CANCEL: return "CANCEL";
    default: return "NONE";
  }
}

void publishSafetyEvents() {
  SafetyEvent event;
  while (safety.popEvent(event)) {
    if (event.type == SafetyEventType::CANE_FALL_SUSPECTED) {
      fallCountdownActive = true;
      fallCountdownStartedAt = event.occurredAtMs;
      lastCountdownPrintAt = event.occurredAtMs;
      Serial.println("[COUNTDOWN] 10 seconds to cancel");
    } else if (event.type == SafetyEventType::CANCEL ||
               event.type == SafetyEventType::CANE_FALL_ALERT) {
      fallCountdownActive = false;
      if (event.type == SafetyEventType::CANCEL) Serial.println("[RESULT] Fall alert cancelled.");
      else Serial.println("[ALERT] Fall countdown expired.");
    }
    char eventId[80];
    char payload[256];
    const uint32_t macSuffix = static_cast<uint32_t>(ESP.getEfuseMac());
    snprintf(eventId, sizeof(eventId), "cane-01-%08lX-%lu",
             static_cast<unsigned long>(macSuffix),
             static_cast<unsigned long>(++eventCounter));
    snprintf(payload, sizeof(payload),
      "{\"event_id\":\"%s\",\"device_id\":\"%s\",\"type\":\"%s\","
      "\"latitude\":null,\"longitude\":null,\"gps_valid\":false}",
      eventId, DEVICE_ID, safetyEventName(event.type));

    Serial.printf("[EVENT] %s\n", payload);
    if (mqtt.connected()) {
      const bool ok = publishMessage(EVENT_TOPIC, payload, 1);
      Serial.printf("[EVENT MQTT] %s QoS1\n", ok ? "sent" : "publish failed");
    } else {
      Serial.println("[EVENT MQTT] offline; local buzzer remains active, event not queued");
    }
  }
}

void printFallCountdown(uint32_t now) {
  if (!fallCountdownActive || now - lastCountdownPrintAt < 1000) return;
  lastCountdownPrintAt = now;
  const uint32_t elapsed = now - fallCountdownStartedAt;
  const int32_t secondsLeft = 10 - static_cast<int32_t>(elapsed / 1000);
  if (secondsLeft > 0) Serial.printf("[COUNTDOWN] %ld seconds left\n", (long)secondsLeft);
}

void sampleSafety(uint32_t now) {
  if (now - lastMpuSample < MPU_SAMPLE_INTERVAL_MS) return;
  lastMpuSample = now;

  float accelerationG = 1.0f;
  float tiltDegrees = 0.0f;
  if (mpuReady) {
    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);
    const float x = accel.acceleration.x;
    const float y = accel.acceleration.y;
    const float z = accel.acceleration.z;
    const float magnitude = sqrtf(x*x + y*y + z*z);
    accelerationG = magnitude / 9.80665f;
    if (magnitude > 0.5f) {
      float dot = (x/magnitude)*uprightX + (y/magnitude)*uprightY + (z/magnitude)*uprightZ;
      dot = constrain(dot, -1.0f, 1.0f);
      tiltDegrees = acosf(dot) * 57.29578f;
    }
  }

  safety.update(digitalRead(Pin::SOS_BUTTON) == LOW, accelerationG, tiltDegrees, now);
  static bool previousBuzzerState = false;
  const bool buzzerOn = safety.buzzerOn();
  if (buzzerOn != previousBuzzerState) {
    if (buzzerOn) tone(Pin::BUZZER_DRIVER, 2000);
    else noTone(Pin::BUZZER_DRIVER);
    previousBuzzerState = buzzerOn;
  }
  publishSafetyEvents();
}

void setup() {
  Serial.begin(115200);
  pinMode(Pin::SOS_BUTTON, INPUT_PULLUP);
  pinMode(Pin::BUZZER_DRIVER, OUTPUT);
  digitalWrite(Pin::BUZZER_DRIVER, LOW);
  Wire.begin(Pin::MPU_SDA, Pin::MPU_SCL);
  mpuReady = mpu.begin(0x68, &Wire);
  if (mpuReady) {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    if (calibrateUpright()) Serial.println("[MPU] upright reference saved");
    else Serial.println("[MPU] calibration incomplete; default reference used");
  } else {
    Serial.println("[MPU] not found; fall detection disabled, local SOS remains available");
  }
  safety.begin(digitalRead(Pin::SOS_BUTTON) == LOW, millis());

  WiFi.mode(WIFI_STA);
  mqtt.setKeepAliveInterval(60000);
  mqtt.setConnectionTimeout(1000); // bounds connect/PUBACK blocking so safety loop resumes quickly
  mqtt.setTxPayloadSize(512);
  if (strlen(MQTT_USERNAME) > 0) mqtt.setUsernamePassword(MQTT_USERNAME, MQTT_PASSWORD);
  const char* willPayload = "offline";
  mqtt.beginWill(STATUS_TOPIC, static_cast<unsigned short>(strlen(willPayload)), true, 0);
  mqtt.print(willPayload);
  mqtt.endWill();
  ensureWifi(millis());
}

void loop() {
  const uint32_t now = millis();
  static bool wasWifiConnected = false;
  const bool connected = WiFi.status() == WL_CONNECTED;
  if (connected && !wasWifiConnected) {
    Serial.print("[WIFI] connected IP=");
    Serial.println(WiFi.localIP());
  }
  if (!connected && wasWifiConnected) Serial.println("[WIFI] lost connection");
  wasWifiConnected = connected;

  if (!connected) ensureWifi(now);
  else {
    ensureMqtt(now);
    if (mqtt.connected()) {
      mqtt.poll();
      publishTelemetry(now);
    }
  }

  // Safety input and local sound continue even while Wi-Fi/MQTT are offline.
  sampleSafety(now);
  printFallCountdown(now);
  delay(5);
}
