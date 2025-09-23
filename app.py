import os
import time
import json
import logging

import paho.mqtt.client as mqtt


# --- Config mínima (1 variable) ---
BROKER = os.getenv("MQTT_BROKER", "localhost")
PORT = 1883
CLIENT_ID = "env-sensors-led"
LED_TOPIC = "SAD/led"
SUB_TOPICS = ["SAD/sps30", "SAD/dht22", "SAD/mq7"]
PUBLISH_INTERVAL_SEC = 1
LOG_LEVEL = "INFO"

logging.basicConfig(level=getattr(logging, LOG_LEVEL, logging.INFO),
                    format='%(asctime)s %(levelname)s %(message)s')
logger = logging.getLogger(__name__)

# Cache de últimas lecturas simples
last = {"pm25": None, "temp": None, "hum": None, "co": None}


def to_float(x):
    try:
        return float(x)
    except Exception:
        return None


def severity_pm25(pm25):
    # 0-15 Normal (green), 15-25 Preocupante (yellow), >25 Grave+ (red)
    if pm25 is None:
        return "green"
    if pm25 <= 15:
        return "green"
    if pm25 <= 25:
        return "yellow"
    return "red"


def severity_temp(t):
    # <20 Fresco (yellow), 20-27 Cómodo (green), 27-32 Precaución (yellow), >32 Peligro (red)
    if t is None:
        return "green"
    if 20 <= t <= 27:
        return "green"
    if t < 20 or (27 < t <= 32):
        return "yellow"
    return "red"


def severity_hum(h):
    # 40-70% (green), 30-40 o 70-90 (yellow), <30 o >90 (red)
    if h is None:
        return "green"
    if 40 <= h <= 70:
        return "green"
    if (30 <= h < 40) or (70 < h <= 90):
        return "yellow"
    return "red"


def severity_co(ppm):
    # <=9 green, <=25 yellow, >25 red (simple)
    if ppm is None:
        return "green"
    if ppm <= 9:
        return "green"
    if ppm <= 25:
        return "yellow"
    return "red"


def worst_color(colors):
    rank = {"green": 0, "yellow": 1, "red": 2}
    return max(colors, key=lambda c: rank.get(c, 0))


def compute_color():
    colors = [
        severity_pm25(last["pm25"]),
        severity_temp(last["temp"]),
        severity_hum(last["hum"]),
        severity_co(last["co"]),
    ]
    return worst_color(colors)


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        logger.info("Connected to MQTT %s:%s", BROKER, PORT)
        for t in SUB_TOPICS:
            client.subscribe(t, qos=0)
            logger.info("Subscribed to %s", t)
    else:
        logger.error("MQTT connect failed rc=%s", rc)


def on_message(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload.decode("utf-8", errors="ignore")
    try:
        data = json.loads(payload)
    except Exception:
        logger.debug("Ignoring non-JSON on %s: %s", topic, payload)
        return

    # Mapear según tópico esperado
    t = topic.lower()
    if "sps30" in t or "pm" in t:
        last["pm25"] = to_float(data.get("PM2.5") or data.get("pm25") or data.get("PM25"))
    elif "dht22" in t or "temp" in t:
        last["temp"] = to_float(data.get("Temperatura") or data.get("temp") or data.get("temperature"))
        last["hum"] = to_float(data.get("Humedad") or data.get("hum") or data.get("humidity"))
    elif "mq7" in t or "co" in t:
        last["co"] = to_float(data.get("CO") or data.get("co"))


def build_client():
    client = mqtt.Client(client_id=CLIENT_ID, clean_session=True)
    client.on_connect = on_connect
    client.on_message = on_message
    client.reconnect_delay_set(min_delay=1, max_delay=30)
    return client


def main():
    client = build_client()
    client.connect_async(BROKER, PORT, keepalive=60)
    client.loop_start()
    logger.info("Started; publishing color to %s every %ss", LED_TOPIC, PUBLISH_INTERVAL_SEC)
    try:
        while True:
            color = compute_color()
            res = client.publish(LED_TOPIC, payload=color, qos=0, retain=False)
            if res.rc == mqtt.MQTT_ERR_SUCCESS:
                logger.info("Published '%s' to %s", color, LED_TOPIC)
            else:
                logger.error("Publish failed rc=%s", res.rc)
            time.sleep(PUBLISH_INTERVAL_SEC)
    except KeyboardInterrupt:
        logger.info("Shutting down...")
    finally:
        try:
            client.loop_stop()
        except Exception:
            pass


if __name__ == "__main__":
    main()
