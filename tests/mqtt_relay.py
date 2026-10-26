
import paho.mqtt.client as mqtt

PUBLIC_BROKER = "broker.hivemq.com"
LOCAL_BROKER = "127.0.0.1"

SOURCE_TOPIC = "smartcane/cane-giang-demo/telemetry"

local_client = mqtt.Client(
    mqtt.CallbackAPIVersion.VERSION2,
    client_id="smartcane-local-relay"
)

public_client = mqtt.Client(
    mqtt.CallbackAPIVersion.VERSION2,
    client_id="smartcane-public-relay"
)


def on_message(client, userdata, msg):
    result = local_client.publish(
        msg.topic,
        msg.payload,
        qos=0,
        retain=False
    )

    if result.rc == mqtt.MQTT_ERR_SUCCESS:
        print("[RELAY] Forwarded:", msg.payload.decode(
            "utf-8", errors="replace"
        ), flush=True)
    else:
        print("[RELAY] Publish failed:", result.rc, flush=True)


def main():
    local_client.connect(LOCAL_BROKER, 1883, 60)
    local_client.loop_start()

    public_client.on_message = on_message
    public_client.connect(PUBLIC_BROKER, 1883, 60)
    public_client.subscribe(SOURCE_TOPIC, qos=0)

    print("[RELAY] Listening:", SOURCE_TOPIC, flush=True)
    public_client.loop_forever()


if __name__ == "__main__":
    main()
