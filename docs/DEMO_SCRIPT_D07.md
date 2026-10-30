# D07 demo run sheet (about 3 minutes)

1. **Show the build and wiring (20 s):** ESP32, MPU6050, SOS switch, buzzer driver, and power source. Point out GPIOs from `docs/PINOUT.md`.
2. **Show startup calibration (15 s):** hold cane upright and still; wait for `[MPU] upright reference saved`.
3. **Show normal motion (20 s):** move the cane normally; show sensor logs without a fall alert.
4. **Show suspected cane fall and cancellation (35 s):** reproduce a controlled drop/tilt safely; press SOS/cancel once in the countdown; show `CANCEL`, no `CANE_FALL_ALERT`.
5. **Show an uncancelled controlled scenario (35 s):** repeat under a safe controlled setup, let countdown end, show `CANE_FALL_ALERT` locally and in backend/Telegram if configured.
6. **Show SOS (25 s):** return to NORMAL, hold the SOS button at least 3 seconds, show one SOS event and sound.
7. **Show GPS and offline behavior (25 s):** use synthetic valid GPS for a Maps notification, then missing GPS; disconnect Wi-Fi and verify local SOS/buzzer still works.
8. **Close with evidence (20 s):** show D06 run report and state that thresholds need real-world tuning and the alert only indicates suspected cane fall.

Keep all fall simulations controlled, use a dummy cane, and do not stage a person falling. If a step has not been run, label it pending rather than presenting it as completed.
