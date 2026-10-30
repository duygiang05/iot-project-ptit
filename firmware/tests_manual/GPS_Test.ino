#include <TinyGPS++.h>

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println("GPS starting...");
}

void loop() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  static unsigned long lastPrint = 0;

  if (millis() - lastPrint >= 1000) {
    lastPrint = millis();

    if (gps.location.isValid() &&
        gps.location.age() < 5000) {
      Serial.print("Latitude: ");
      Serial.println(gps.location.lat(), 6);

      Serial.print("Longitude: ");
      Serial.println(gps.location.lng(), 6);
    } else {
      Serial.println("Waiting for GPS fix...");
    }
  }
}
