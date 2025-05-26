# flask_mqtt_server.py
import pickle, cv2, numpy as np, face_recognition
import paho.mqtt.client as mqtt
from flask import Flask, request, jsonify

# —— 设定区 —— #
CACHE_PATH = "known_faces_cache.pkl"
THRESHOLD  = 0.6
MQTT_BROKER = "test.mosquitto.org"
MQTT_PORT   = 1883
MQTT_TOPIC  = "face/motor/control"
# ———————— #

# 1) 预加载人脸编码
with open(CACHE_PATH, "rb") as f:
    cache = pickle.load(f)
known_encodings = cache["encodings"]
known_names     = cache["names"]

# 2) 建立 MQTT 并保持 loop
mqttc = mqtt.Client()
mqttc.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
mqttc.loop_start()

# 3) Flask 应用
app = Flask(__name__)

@app.route("/recognize", methods=["POST"])
def recognize():
    file = request.files.get("image")
    if not file:
        return jsonify({"result":"ERROR","msg":"no image"}), 400

    # 从字节解码到 OpenCV BGR
    arr = np.frombuffer(file.read(), np.uint8)
    img = cv2.imdecode(arr, cv2.IMREAD_COLOR)
    rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

    # 检测与编码
    locs = face_recognition.face_locations(rgb)
    encs = face_recognition.face_encodings(rgb, locs)
    if not encs:
        return jsonify({"result":"FAIL","msg":"no face"}), 200

    # 只取第一张脸
    enc = encs[0]
    dists = face_recognition.face_distance(known_encodings, enc)
    idx   = int(np.argmin(dists))
    best  = float(dists[idx])

    if best <= THRESHOLD:
        name, cmd = known_names[idx], "1"
    else:
        name, cmd = "Unknown", "0"

    # 发布到 MQTT
    mqttc.publish(MQTT_TOPIC, cmd)

    # 回传给 ESP32
    return jsonify({
      "result":"OK",
      "name": name,
      "distance": best,
      "cmd": cmd
    })

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
