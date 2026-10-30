#!/usr/bin/env python3
"""Repeated local MQTT -> Spring API integration checks for D06."""
import argparse
import json
import time
import urllib.error
import urllib.request
import uuid
from datetime import datetime, timezone
from pathlib import Path

try:
    import paho.mqtt.client as mqtt
except ImportError as exc:
    raise SystemExit("Install dependency first: py -m pip install paho-mqtt") from exc


def get_incidents(api_base, device_id):
    url = f"{api_base.rstrip('/')}/api/devices/{device_id}/incidents?limit=100"
    with urllib.request.urlopen(url, timeout=3) as response:
        return json.load(response)


def wait_for_event(api_base, device_id, event_id, timeout_s):
    deadline = time.monotonic() + timeout_s
    while time.monotonic() < deadline:
        try:
            rows = get_incidents(api_base, device_id)
            match = next((row for row in rows if row.get("eventId") == event_id), None)
            if match:
                return match
        except (urllib.error.URLError, TimeoutError, json.JSONDecodeError):
            pass
        time.sleep(0.1)
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--broker", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=1883)
    parser.add_argument("--api", default="http://127.0.0.1:8081")
    parser.add_argument("--repeat", type=int, default=5)
    parser.add_argument("--timeout", type=float, default=8.0)
    parser.add_argument("--device-id", default="d06-test")
    parser.add_argument("--output", default="tests/d06-results.json")
    parser.add_argument("--include-notification-cases", action="store_true",
                        help="Also publish SOS/FALL_ALERT; sends messages if Telegram is enabled on backend")
    args = parser.parse_args()
    if args.repeat < 1 or args.repeat > 100:
        parser.error("--repeat must be between 1 and 100")

    scenarios = [
        ("CANE_FALL_SUSPECTED", False, None, None),
        ("CANCEL", False, None, None),
    ]
    if args.include_notification_cases:
        scenarios += [
            ("CANE_FALL_ALERT", False, None, None),
            ("CANE_FALL_ALERT", True, 10.762622, 106.660172),
            ("SOS", False, None, None),
            ("SOS", True, 10.762622, 106.660172),
        ]

    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2,
                         client_id=f"d06-{uuid.uuid4().hex[:10]}")
    client.connect(args.broker, args.port, keepalive=30)
    client.loop_start()
    rows = []
    run_id = uuid.uuid4().hex[:8]
    try:
        for event_type, gps_valid, latitude, longitude in scenarios:
            for iteration in range(1, args.repeat + 1):
                event_id = f"{args.device_id}-{run_id}-{len(rows)+1}"
                payload = {
                    "event_id": event_id,
                    "device_id": args.device_id,
                    "type": event_type,
                    "gps_valid": gps_valid,
                    "latitude": latitude,
                    "longitude": longitude,
                }
                topic = f"smartcane/{args.device_id}/events"
                start = time.monotonic()
                info = client.publish(topic, json.dumps(payload), qos=1, retain=False)
                info.wait_for_publish(timeout=args.timeout)
                published = info.is_published() and info.rc == mqtt.MQTT_ERR_SUCCESS
                incident = wait_for_event(args.api, args.device_id, event_id, args.timeout) if published else None
                elapsed_ms = round((time.monotonic() - start) * 1000, 1)
                rows.append({
                    "case": f"{event_type}_gps_{'valid' if gps_valid else 'missing'}",
                    "iteration": iteration,
                    "event_id": event_id,
                    "published_qos1": published,
                    "backend_saved": incident is not None,
                    "mqtt_to_api_ms": elapsed_ms if incident else None,
                    "telegram_expected": event_type in ("SOS", "CANE_FALL_ALERT"),
                    "pass": published and incident is not None,
                })
                print(f"{rows[-1]['case']} #{iteration}: "
                      f"{'PASS' if rows[-1]['pass'] else 'FAIL'} "
                      f"api_ms={rows[-1]['mqtt_to_api_ms']}")

        # Verify event_id idempotency by publishing one exact payload twice.
        duplicate_payload = {
            "event_id": f"{args.device_id}-{run_id}-duplicate",
            "device_id": args.device_id,
            "type": "CANCEL",
            "gps_valid": False,
            "latitude": None,
            "longitude": None,
        }
        topic = f"smartcane/{args.device_id}/events"
        for _ in range(2):
            client.publish(topic, json.dumps(duplicate_payload), qos=1).wait_for_publish(timeout=args.timeout)
        time.sleep(0.5)
        matches = [r for r in get_incidents(args.api, args.device_id)
                   if r.get("eventId") == duplicate_payload["event_id"]]
        rows.append({"case": "duplicate_event_id", "event_id": duplicate_payload["event_id"],
                     "stored_count": len(matches), "pass": len(matches) == 1})
    finally:
        client.loop_stop()
        client.disconnect()

    report = {
        "generated_at_utc": datetime.now(timezone.utc).isoformat(),
        "broker": f"{args.broker}:{args.port}",
        "api": args.api,
        "device_id": args.device_id,
        "repeat": args.repeat,
        "notification_cases_enabled": args.include_notification_cases,
        "summary": {"passed": sum(bool(row["pass"]) for row in rows),
                    "total": len(rows), "all_pass": all(bool(row["pass"]) for row in rows)},
        "rows": rows,
        "limitations": [
            "mqtt_to_api_ms measures persistence visibility, not Telegram delivery.",
            "For Telegram latency, collect backend [TELEGRAM] end_to_end_ms logs.",
            "Physical offline/local buzzer testing is not represented by this MQTT script.",
        ],
    }
    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"Summary: {report['summary']['passed']}/{report['summary']['total']} passed; report={output}")
    raise SystemExit(0 if report["summary"]["all_pass"] else 1)


if __name__ == "__main__":
    main()
