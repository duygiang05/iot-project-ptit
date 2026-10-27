# Duy — D01/D02: MPU6050, suspected cane fall, SOS and buzzer

This sketch reads the MPU6050 through the Adafruit driver. On real hardware, hold the cane upright and still during its 2-second startup calibration. Values in the sketch are prototype thresholds and need tuning against measurements from the assembled cane.

## Wokwi

1. Copy `sketch.ino` and `diagram.json` into an ESP32 Wokwi project.
2. Copy the library list from `libraries.txt` (or add **Adafruit MPU6050** through Wokwi Library Manager; its Adafruit dependencies are listed here).
3. Start the simulation and open Serial Monitor at 115200 baud.
4. To simulate a cane impact, set MPU6050 `accelX` to about `2.0` g and `accelZ` to `0`. Then set `accelX` to about `0.9` g and `accelZ` to `0.3` g and keep it there. The sketch should detect an impact followed by a stable tilt over 60 degrees, start the 10-second countdown and then issue `CANE_FALL_ALERT` unless you press the button.
5. Hold the button for 3 seconds in NORMAL to test SOS. Press once during countdown to cancel.

The MPU6050 controls can be changed through Wokwi automation scenarios. This sketch prints event JSON on Serial; it does not publish MQTT. A suspected cane fall does not confirm that a person has fallen.

## Wiring

MPU6050: VCC to 3V3, GND to GND, SDA to D21, SCL to D22. Button: GPIO13 to one side and GND to the other. Buzzer: GPIO25 to positive, GND to negative. On a physical build, use a transistor/driver if the buzzer draws more current than an ESP32 GPIO can safely supply.
