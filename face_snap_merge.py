import time
import requests
import os
import pickle
import cv2
import numpy as np
import face_recognition
import paho.mqtt.client as mqtt
from playsound import playsound
import pygame

pygame.mixer.init()

# ———— 設定區 ————
CACHE_PATH  = "known_faces_cache.pkl"
UNKNOWN_DIR = "verify"
THRESHOLD   = 0.45

MQTT_BROKER = "test.mosquitto.org"
MQTT_PORT   = 1883
MQTT_TOPIC  = "face/motor/control"
MQTT_BUTTON_TOPIC = "face/button/press"
# ——————————————————


with open(CACHE_PATH, "rb") as f:
    cache = pickle.load(f)
known_encodings = cache["encodings"]
known_names     = cache["names"]

# 2. 建立 MQTT 連線
client = mqtt.Client()
# client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
# client.subscribe(MQTT_BUTTON_TOPIC)



# ESP32-CAM 的 AP 模式或 STA IP
ESP_IP = "192.168.10.41"        # 看你們的serial monitor上是什麼
URL    = f"http://{ESP_IP}/capture"
#SAVE_DIR = r"C:\Users\User\Documents\GitHub\doorLock\verify" #我直接存在verify裡面
SAVE_DIR = "verify" #應該可以直接這樣寫

os.makedirs(SAVE_DIR, exist_ok=True)

sound_familymart = pygame.mixer.Sound("familymart.wav")
sound_why_gay = pygame.mixer.Sound("Why are you Gay.wav")
sound_u_r_gay = pygame.mixer.Sound("u r gay.wav")
sound_button_press = pygame.mixer.Sound("seeyouagain.wav")
sound_failure = pygame.mixer.Sound("failure.wav")
sound_haiyaa = pygame.mixer.Sound("Haiyaa.wav")
sound_stillfail = pygame.mixer.Sound("stillfailure.wav")

def on_connect(client, userdata, flags, rc):
    client.subscribe(MQTT_BUTTON_TOPIC)    # 訂閱新的按鈕 topic

def on_message(client, userdata, msg):
    #client.subscribe(MQTT_BUTTON_TOPIC)
    print(f"Message received - Topic: {msg.topic}, Payload: {msg.payload.decode()}")
    if msg.topic == MQTT_BUTTON_TOPIC:
        if msg.payload.decode() == "p": # 如果 payload 是 "pressed"
            print("Button pressed! Playing sound...") # 輸出訊息
            #playsound("seeyouagain.wav", block=True) # 播放提示音效
            sound_button_press.play()
            cmd = "1"
            client.publish(MQTT_TOPIC, payload=cmd, qos=0, retain=False)
            print(f"[MQTT] Published {cmd} to {MQTT_TOPIC}")
            cmd = "0"

client.on_connect = on_connect # 設定 on_connect 回呼函數
client.on_message = on_message # 設定 on_message 回呼函數

client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)


client.loop_start()

idx = 0
fail = 0
while True:
    try:
        resp = requests.get(URL, timeout=5)
        if resp.status_code == 200:
            '''
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
            '''

            img_arr = np.frombuffer(resp.content, np.uint8)
            image   = cv2.imdecode(img_arr, cv2.IMREAD_COLOR)
            rgb     = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

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
                    cmd = "1"   
                    #playsound("familymart.wav", block=False)
                    sound_familymart.play()
                    fail = 0
                    
                else:
                    fail = fail + 1
                    cmd = "0"
                    if fail % 3 == 1:
                        #playsound("Why are you Gay.wav", block=False)
                        sound_haiyaa.play()
                    if fail % 3 == 2:
                        #playsound("u r gay.wav", block=False)
                        sound_failure.play()
                    if fail % 3 == 0:
                        #playsound("u r gay.wav", block=False)
                        sound_stillfail.play()                    

                # 發佈 MQTT
                client.publish(MQTT_TOPIC, payload=cmd, qos=0, retain=False)
                print(f"[MQTT] Published {cmd} to {MQTT_TOPIC}")

                # 畫框顯示

                color = (0,255,0) if name != "Unknown" else (0, 0, 255)
                
                cv2.rectangle(image, (left, top), (right, bottom), color, 2)
                cv2.putText(image, name, (left, top-10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.8, color, 2)
                
                filename = os.path.join(SAVE_DIR, f"snap_{idx:04d}.jpg")
                cv2.imwrite(filename, image)
                idx += 1   

                print(f"偵測到 {name}")
                print(distances[best_idx])

                

            cv2.imshow(filename, image)
            # cv2.waitKey(0)
            cv2.waitKey(1000)
            cv2.destroyAllWindows()

            if(cmd=="1"):
                time.sleep(5)

            #if cv2.waitKey(1000) & 0xFF == ord('q'):
               # break     

            if not client.is_connected():
                client.reconnect()

        else:
            print("HTTP", resp.status_code)
    except Exception as e:
        print("Error:", e)
    #time.sleep(3)  # 每過4秒拍一張
    #rrr
