# Smart Cane — D03–D07 completion report

**Team/member:** Pending
**Board revision:** Pending
**Test date/time and timezone:** Pending
**Firmware commit:** Pending
**Backend commit:** Pending
**Wokwi project link:** Pending

## D03 — Hardware setup and threshold tuning

| Item | Actual connection/value | Evidence / notes |
|---|---|---|
| MPU6050 VCC/GND/SDA/SCL | 3V3 / GND / GPIO21 / GPIO22 | Pending |
| SOS button | GPIO13 to GND, INPUT_PULLUP | Pending |
| Buzzer driver | GPIO14 to driver input | Pending |
| Upright calibration | 2 seconds at boot | Pending |
| Free-fall threshold | 0.55 g (initial) → measured: ___ | Pending |
| Impact threshold | 1.80 g (initial) → measured: ___ | Pending |
| Tilt threshold | 60 degrees (initial) → measured: ___ | Pending |

Attach pinout photo, sensor serial log, and Wokwi-vs-hardware comparison. State board, sensor orientation, sample count, and how each threshold was selected.

## D04 — Telegram delivery

| Event | GPS state | Expected message | Actual | Result/evidence |
|---|---|---|---|---|
| SOS | Missing | SOS + GPS unavailable | Pending | Pending |
| SOS | Valid synthetic coordinates | SOS + Maps link | Pending | Pending |
| CANE_FALL_ALERT | Valid / missing | One alert, map only if valid | Pending | Pending |
| CANCEL / suspected | Any | No Telegram alert | Pending | Pending |
| Duplicate event_id | Any | No duplicate notification | Pending | Pending |

Do not include bot token, private chat ID, or real personal location in the report.

## D05 — Test cases

| Case | Runs | Expected | Actual | Pass/fail | Log/photo |
|---|---:|---|---|---|---|
| SOS button hold | 5 | One SOS per hold | Pending | Pending | Pending |
| Impact + stable tilt | 5 | Suspected then countdown | Pending | Pending | Pending |
| Cancel during countdown | 5 | CANCEL, no fall alert | Pending | Pending | Pending |
| Countdown expires | 5 | CANE_FALL_ALERT | Pending | Pending | Pending |
| GPS missing/invalid | 5 | No fabricated coordinates | Pending | Pending | Pending |
| Duplicate event ID | 5 | One persisted incident | Pending | Pending | Pending |

## D06 — Integration measures

| Run | Device event to backend API (ms) | Backend to Telegram (ms) | Wi-Fi/GPS conditions | Result/evidence |
|---:|---:|---:|---|---|
| 1 | Pending | Pending | Pending | Pending |
| 2 | Pending | Pending | Pending | Pending |
| 3 | Pending | Pending | Pending | Pending |
| 4 | Pending | Pending | Pending | Pending |
| 5 | Pending | Pending | Pending | Pending |

Report median, minimum, maximum, and failures from actual measurements. Include at least one no-GPS run and one offline local SOS run.

## D07 — Demo and known limitations

**Demo video/photo link:** Pending
**What the demo proves:** Pending
**Known bugs:** Pending
**Thresholds not yet validated:** Pending
**Safety statement:** This prototype detects suspected cane-fall patterns; it does not establish that a person has fallen and is not a certified medical or emergency device.
