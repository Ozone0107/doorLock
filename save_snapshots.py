import time
import requests
import os

# ESP32-CAM 的 AP 模式或 STA IP
ESP_IP = "172.20.10.2"        # 看你們的serial monitor上是什麼
URL    = f"http://{ESP_IP}/capture"
SAVE_DIR = r"C:\Users\User\Documents\GitHub\doorLock\verify" #我直接存在verify裡面

os.makedirs(SAVE_DIR, exist_ok=True)

idx = 0
while True:
    try:
        resp = requests.get(URL, timeout=5)
        if resp.status_code == 200:
            filename = os.path.join(SAVE_DIR, f"snap_{idx:04d}.jpg")
            with open(filename, "wb") as f:
                f.write(resp.content)
            print(f"Saved {filename}")
            idx += 1
        else:
            print("HTTP", resp.status_code)
    except Exception as e:
        print("Error:", e)
    time.sleep(5)  # 每過5秒拍一張
