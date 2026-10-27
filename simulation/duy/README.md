# Wokwi D01/D02: Fake fall warning, SOS and buzzer

This sketch uses deterministic fake sensor values so the warning flow can be demonstrated without relying on MPU6050 readings. The MPU6050 remains in the diagram as the intended hardware component; the sketch does not read it in fake-data mode.

## Run

1. Open an ESP32 project in Wokwi.
2. Copy `sketch.ino` into the sketch editor and replace `diagram.json` with this folder's diagram.
3. Start the simulation and open Serial Monitor at 115200 baud.
4. Enter `F` in the Serial input and send it. The sketch prints fake acceleration/tilt, `CANE_FALL_SUSPECTED`, a 10-second countdown, then `CANE_FALL_ALERT` and sounds the buzzer.
5. During the countdown, enter `C` or press the green button once to cancel. Enter `R` to reset and repeat.
6. To test SOS, enter `S` or hold the green button for 3 seconds while state is NORMAL.

Serial commands: `F` fake fall, `C` cancel countdown, `S` SOS, `R` reset. Event lines are JSON-shaped Serial output for demonstration; this sketch does not publish MQTT. The fall warning means suspected cane fall only, not a confirmed person fall.

No external Arduino libraries are required for this deterministic fake-data sketch. The MPU6050 is shown in the circuit but its values are simulated by the `F` command.
