import paho.mqtt.client as mqtt
 
MQTT_BROKER = "test.mosquitto.org"
TOPIC_UID   = "rfid/uid"
TOPIC_STATUS= "rfid/status"
 
def on_connect(client, userdata, flags, rc):
    print("✅ Connected to MQTT broker with result code", rc)

    client.subscribe(TOPIC_UID)
    client.subscribe(TOPIC_STATUS)
    print(f"🔔 Subscribed to {TOPIC_UID} and {TOPIC_STATUS}")
 
def on_message(client, userdata, msg):
    payload = msg.payload.decode()
    print(f"📬 Topic: {msg.topic}   Payload: {payload}")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
 

client.connect(MQTT_BROKER, 1883, keepalive=60)
client.loop_start()
 
print("📡 Waiting for RFID messages...")
 

import time
while True:
    time.sleep(1)