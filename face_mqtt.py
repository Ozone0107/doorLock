import os
import pickle
import cv2
import numpy as np
import face_recognition
import paho.mqtt.client as mqtt

# ———— 設定區 ————
CACHE_PATH  = "known_faces_cache.pkl"
UNKNOWN_DIR = "verify"
THRESHOLD   = 0.6

MQTT_BROKER = "test.mosquitto.org"
MQTT_PORT   = 1883
MQTT_TOPIC  = "face/motor/control"
# ——————————————————

# 1. 載入快取
with open(CACHE_PATH, "rb") as f:
    cache = pickle.load(f)
known_encodings = cache["encodings"]
known_names     = cache["names"]

# 2. 建立 MQTT 連線
client = mqtt.Client()
client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
client.loop_start()

# 3. 逐張辨識並發布命令
for filename in os.listdir(UNKNOWN_DIR):
    img_path = os.path.join(UNKNOWN_DIR, filename)
    image    = cv2.imread(img_path)
    rgb      = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

    # 辨識
    locs     = face_recognition.face_locations(rgb)
    encodings= face_recognition.face_encodings(rgb, locs)

    for (top, right, bottom, left), enc in zip(locs, encodings):
        distances = face_recognition.face_distance(known_encodings, enc)
        best_idx  = np.argmin(distances)
        name = "Unknown"
        best_dist = distances[best_idx]

        if best_dist <= THRESHOLD:
            name = known_names[best_idx]
            cmd = "1"   # Known → 轉動馬達
        else:
            cmd = "0"   # Unknown → 停止馬達

        # 發佈 MQTT
        client.publish(MQTT_TOPIC, payload=cmd, qos=0, retain=False)
        print(f"[MQTT] Published {cmd} to {MQTT_TOPIC}")

        # 畫框顯示
        cv2.rectangle(image, (left, top), (right, bottom), (0,255,0), 2)
        cv2.putText(image, name, (left, top-10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0,255,0), 2)
        print(f"偵測到 {name}")
        print(distances[best_idx])

    cv2.imshow(filename, image)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

# 4. 清理
client.loop_stop()
client.disconnect()
