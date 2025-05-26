import time
import requests
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


with open(CACHE_PATH, "rb") as f:
    cache = pickle.load(f)
known_encodings = cache["encodings"]
known_names     = cache["names"]

# 2. 建立 MQTT 連線
client = mqtt.Client()
client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
client.loop_start()


# ESP32-CAM 的 AP 模式或 STA IP
ESP_IP = "172.20.10.2"        # 看你們的serial monitor上是什麼
URL    = f"http://{ESP_IP}/capture"
#SAVE_DIR = r"C:\Users\User\Documents\GitHub\doorLock\verify" #我直接存在verify裡面
SAVE_DIR = "verify" #應該可以直接這樣寫

os.makedirs(SAVE_DIR, exist_ok=True)

idx = 0
while True:
    try:
        resp = requests.get(URL, timeout=5)
        if resp.status_code == 200:
            #filename = os.path.join(SAVE_DIR, f"snap_{idx:04d}.jpg")
            snap_name = f"snap_{idx:04d}.jpg"
            filename = os.path.join(SAVE_DIR, snap_name)
            with open(filename, "wb") as f:
                f.write(resp.content)
            print(f"Saved {filename}")
            idx += 1
            #for filename in os.listdir(UNKNOWN_DIR):
                #img_path = os.path.join(UNKNOWN_DIR, filename)
            img_path = filename
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

                color = (0,255,0) if name != "Unknown" else (0, 0, 255)
                
                cv2.rectangle(image, (left, top), (right, bottom), color, 2)
                cv2.putText(image, name, (left, top-10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.8, color, 2)
                print(f"偵測到 {name}")
                print(distances[best_idx])

            cv2.imshow(filename, image)
            # cv2.waitKey(0)
            # cv2.destroyAllWindows()

            if cv2.waitKey(1000) & 0xFF == ord('q'):
                break     

            if not client.is_connected():
                client.reconnect()

        else:
            print("HTTP", resp.status_code)
    except Exception as e:
        print("Error:", e)
    time.sleep(5)  # 每過5秒拍一張

