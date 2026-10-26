#include <WiFi.h>
#include <PubSubClient.h>

// Public MQTT broker - only synthetic/non-sensitive values. No credentials here.
static const char* WIFI_SSID = "Wokwi-GUEST";
static const char* MQTT_HOST = "broker.hivemq.com";
static constexpr uint16_t MQTT_PORT = 1883;
static const char* DEVICE_ID = "cane-giang-demo";
static const char* TOPIC = "smartcane/cane-giang-demo/telemetry";
WiFiClient wifi;
PubSubClient mqtt(wifi);
uint32_t lastWifiTry=0, lastMqttTry=0, lastSend=0;
uint32_t seq=0;

void connectWifi(uint32_t now) {
  if (WiFi.status() == WL_CONNECTED || now - lastWifiTry < 5000) return;
  lastWifiTry = now;
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, "", 6);
  Serial.println("[WIFI] connecting...");
}
void connectMqtt(uint32_t now) {
  if (WiFi.status()!=WL_CONNECTED || mqtt.connected() || now-lastMqttTry<3000) return;
  lastMqttTry=now;
  String clientId = String("wokwi-")+DEVICE_ID+"-"+String(random(0x7fffffff), HEX);
  if(mqtt.connect(clientId.c_str())) Serial.println("[MQTT] connected");
  else { Serial.print("[MQTT] failed state="); Serial.println(mqtt.state()); }
}
void sendMock(uint32_t now) {
  if(!mqtt.connected() || now-lastSend<5000) return;
  lastSend=now;
  float distance = 45.0f + (seq++ % 10) * 10.0f;
  // No real clock in this starter: omit timestamp (backend adds receivedAt).
  char payload[256];
  snprintf(payload, sizeof(payload),
      "{\"device_id\":\"%s\",\"distance_cm\":%.1f,\"latitude\":null,\"longitude\":null,\"gps_valid\":false,\"water_detected\":false,\"light_level\":1200,\"led_on\":false,\"battery_voltage\":null}",
      DEVICE_ID, distance);
  bool ok = mqtt.publish(TOPIC,payload,false);
  Serial.printf("[PUBLISH] %s %s\n",ok?"OK":"FAILED",payload);
}
void setup() {
 Serial.begin(115200);
 randomSeed(micros());
 mqtt.setServer(MQTT_HOST,MQTT_PORT);
 mqtt.setBufferSize(512);
 WiFi.mode(WIFI_STA);
 WiFi.begin(WIFI_SSID,"",6);
 Serial.println("Wokwi WiFi / MQTT mock telemetry");
}
void loop() {
 uint32_t now=millis();
 connectWifi(now);
 connectMqtt(now);
 mqtt.loop();
 sendMock(now);
 delay(5);
}
