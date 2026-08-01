#!/usr/bin/env python3
"""
mqtt_replay.py — Re-publish a simulated MQTT diagnostic report.

Re-sends the same JSON report format that the device's `mqtt-pub`
command publishes, without touching the hardware.  Useful for
verifying a subscriber (MQTT Panel / Dash / Node-RED) or for
dashboard demos.

Usage:
    python3 tools/mqtt_replay.py                    # default broker + topic
    python3 tools/mqtt_replay.py --topic mytopic
    python3 tools/mqtt_replay.py --broker mqtt://host:1883
    python3 tools/mqtt_replay.py --file report.json # custom payload
    python3 tools/mqtt_replay.py --listen           # subscribe instead

Requires: pip install paho-mqtt
"""

import argparse
import json
import sys
import time

import paho.mqtt.client as mqtt

DEFAULT_BROKER = "broker.emqx.io"
DEFAULT_PORT = 1883
DEFAULT_TOPIC = "diagtest"

DEFAULT_REPORT = {
    "app": "m5s3_diag",
    "idf": "v6.0.2-563-g89822381923",
    "mac": "a4:cf:12:34:56:78",
    "wifi": {"ssid": "MyWiFi", "ip": "192.168.1.42",
             "rssi": -59, "channel": 11},
    "rtc": "2026-08-01 00:26:29",
    "tests": [
        {"id": "wifi", "result": "PASSED", "elapsed_ms": 3329,
         "message": "connected, IP 192.168.1.42, RSSI -59 dBm (good)"}
    ],
    "errors": [],
    "summary": {"total": 1, "passed": 1, "skipped": 0, "failed": 0},
}


def parse_broker(arg: str) -> tuple:
    """Accept 'host' or 'mqtt://host:port'."""
    s = arg.replace("mqtt://", "")
    host, _, port = s.partition(":")
    return host, int(port) if port else DEFAULT_PORT


def do_publish(args) -> None:
    host, port = parse_broker(args.broker)
    payload = args.file
    if payload:
        with open(payload, "r", encoding="utf-8") as f:
            data = json.load(f)
    else:
        data = DEFAULT_REPORT
    body = json.dumps(data, ensure_ascii=False)

    c = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    c.connect(host, port, 10)
    c.loop_start()
    info = c.publish(args.topic, body, qos=1)
    print(f"Published {len(body)} bytes to {host}:{port}/{args.topic} "
          f"(msg_id={info.mid}, rc={info.rc})")
    time.sleep(3)  # allow QoS 1 PUBACK to flush
    c.loop_stop()
    c.disconnect()
    print("Done — check your subscriber.")


def do_listen(args) -> None:
    host, port = parse_broker(args.broker)

    def on_message(_client, _userdata, msg):
        print(f"[{msg.topic}] {msg.payload.decode('utf-8', 'replace')}")

    c = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    c.on_message = on_message
    c.connect(host, port, 10)
    c.subscribe(args.topic)
    print(f"Listening on {host}:{port}/{args.topic} (Ctrl+C to stop)")
    try:
        c.loop_forever()
    except KeyboardInterrupt:
        pass


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--broker", default=DEFAULT_BROKER,
                   help=f"broker host or mqtt://host:port (default: {DEFAULT_BROKER}:{DEFAULT_PORT})")
    p.add_argument("--topic", default=DEFAULT_TOPIC,
                   help=f"MQTT topic (default: {DEFAULT_TOPIC})")
    p.add_argument("--file", default=None,
                   help="JSON file to publish instead of the default report")
    p.add_argument("--listen", action="store_true",
                   help="subscribe to the topic instead of publishing")
    args = p.parse_args()

    if args.listen:
        do_listen(args)
    else:
        do_publish(args)


if __name__ == "__main__":
    sys.exit(main())
