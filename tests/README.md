# Test fixtures and evidence

- `telemetry.sample.json` and `event.sample.json` are MQTT contract examples.
- `safety-events.jsonl` and `SAFETY_SCENARIOS.md` cover D05 safety events, GPS states, invalid payloads, and duplicate delivery.
- `SafetyModuleTest.cpp` exercises SOS debounce, impact/tilt confirmation, cancel, and alert using a native C++ compiler.
- `d06_integration.py` repeats MQTT-to-API scenarios and writes measured API visibility latency to `tests/d06-results.json`.

Install the runner dependency with `py -m pip install -r tests/requirements.txt`. Start Mosquitto, PostgreSQL, and the backend before `py tests/d06_integration.py --repeat 5`. Add `--include-notification-cases` only when the backend targets a test Telegram chat; SOS and fall alert cases can send real messages.

For evidence, save steps, expected/actual output, pass/fail, timestamp/timezone, logs/photos, Wokwi link, and unresolved issues. Do not include secrets or real personal GPS coordinates.
