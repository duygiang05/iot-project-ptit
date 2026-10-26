#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "WifiSecrets.h"
#include "TelemetryData.h"

static const char* DEVICE_ID = "cane-01";
static const char* TELEMETRY_TOPIC = "smartcane/cane-01/telemetry";
static const char* STATUS_TOPIC = "smartcane/cane-01/status";
static constexpr unsigned long SEND_INTERVAL_MS = 5000;
static constexpr unsigned long RETRY_INTERVAL_MS = 5000;

WiFiClient network;
PubSubClient mqtt(network);
unsigned long lastWifiAttempt = 0;
unsigned long lastMqttAttempt = 0;
unsigned long lastSend = 0;
unsigned long seq = 0;

// To be supplied by Tung, Duy, Dat in G09.
TelemetryData readTelemetry() {
  TelemetryData data;
  data.distanceCm = 45.0f + static_cast<float>((seq++ % 10) * 10);
  return data;
}

void ensureWifi(unsigned long now) {
  if (WiFi.status() == WL_CONNECTED) return;
  if (lastWifiAttempt != 0 && now - lastWifiAttempt < RETRY_INTERVAL_MS) return;
  lastWifiAttempt = now;
  Serial.printf("[WIFI] connecting to %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void ensureMqtt(unsigned long now) {
  if (WiFi.status() != WL_CONNECTED || mqtt.connected()) return;
  if (lastMqttAttempt != 0 && now - lastMqttAttempt < RETRY_INTERVAL_MS) return;
  lastMqttAttempt = now;
  String clientId = String("smartcane-esp32-") + WiFi.macAddress();
  clientId.replace(":", "");
  Serial.printf("[MQTT] trying %s:%d\n", MQTT_HOST, MQTT_PORT);
  if (mqtt.connect(clientId.c_str(), STATUS_TOPIC, 0, true, "offline")) {
    Serial.println("[MQTT] connected");
    mqtt.publish(STATUS_TOPIC, "online", true);
  } else {
    Serial.printf("[MQTT] failed state=%d\n", mqtt.state());
  }
}

void publishTelemetry(unsigned long now) {
  if (!mqtt.connected() || now - lastSend < SEND_INTERVAL_MS) return;
  lastSend = now;
  TelemetryData d = readTelemetry();
  char json[320];
  // Keep GPS null while no module is attached. Timestamp intentionally omitted:
  // Spring Boot sets received_at; it accepts missing device_timestamp (G05 tested).
  snprintf(json, sizeof(json),
    "{\"device_id\":\"%s\",\"distance_cm\":%.1f,\"latitude\":null,"
    "\"longitude\":null,\"gps_valid\":false,\"water_detected\":%s,"
    "\"light_level\":%d,\"led_on\":%s,\"battery_voltage\":null}",
    DEVICE_ID, d.distanceCm, d.waterDetected ? "true" : "false",
    d.lightLevel, d.ledOn ? "true" : "false");
  bool ok = mqtt.publish(TELEMETRY_TOPIC, json, false);
  Serial.printf("[PUBLISH] %s %s\n", ok ? "OK" : "FAILED", json);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("=== Smart Cane G06: real ESP32 Wi-Fi/MQTT ===");
  WiFi.mode(WIFI_STA);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setBufferSize(512);
  mqtt.setSocketTimeout(3);
  ensureWifi(millis());
}

void loop() {
  unsigned long now = millis();
  static bool wasWifiConnected = false;
  bool connected = (WiFi.status() == WL_CONNECTED);
  if (connected && !wasWifiConnected) {
    Serial.print("[WIFI] connected IP=");
    Serial.println(WiFi.localIP());
  }
  if (!connected && wasWifiConnected) Serial.println("[WIFI] lost connection");
  wasWifiConnected = connected;
  if (!connected) {
    ensureWifi(now);
    delay(20);
    return;
  }
  ensureMqtt(now);
  if (mqtt.connected()) {
    mqtt.loop();
    publishTelemetry(now);
  }
  delay(10);
}
