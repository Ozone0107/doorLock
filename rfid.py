import paho.mqtt.client as mqtt
 
def on_connect(client, userdata, flags, rc):
    print("✅ Connected to MQTT broker")
    client.subscribe("rfid/uid")
    client.subscribe("rfid/status")
 
def on_message(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload.decode()
    print(f"[{topic}] {payload}")
 
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
 
# 連到你的 MQTT broker，例如 test.mosquitto.org（或你自己建的）
client.connect("test.mosquitto.org", 1883, 60)
 
print("📡 Waiting for RFID messages...")
client.loop_forever()