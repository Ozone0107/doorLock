import os
import pickle
import cv2
import numpy as np
import face_recognition

CACHE_PATH = "known_faces_cache.pkl"
UNKNOWN_DIR = "verify"
THRESHOLD = 0.6

# 1. 從快取檔讀取
with open(CACHE_PATH, "rb") as f:
    cache = pickle.load(f)
known_encodings = cache["encodings"]
known_names     = cache["names"]

# 2. 逐張辨識
for filename in os.listdir(UNKNOWN_DIR):
    img_path = os.path.join(UNKNOWN_DIR, filename)
    image = cv2.imread(img_path)
    rgb   = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

    locations = face_recognition.face_locations(rgb)
    encodings = face_recognition.face_encodings(rgb, locations)

    for (top, right, bottom, left), face_encoding in zip(locations, encodings):
        distances = face_recognition.face_distance(known_encodings, face_encoding)
        best_idx = np.argmin(distances)
        name = "Unknown"
        if distances[best_idx] <= THRESHOLD:
            name = known_names[best_idx]
            confidence = max(0.0, (THRESHOLD - distances[best_idx]) / THRESHOLD)
            confidence_pct = confidence * 100
            #print(f"偵測到 {name}")
            #print(distances[best_idx])
            #print(f"偵測到 {name}，信心水準：{confidence_pct:.1f}%")

        cv2.rectangle(image, (left, top), (right, bottom), (0,255,0), 2)
        cv2.putText(image, name, (left, top-10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0,255,0), 2)
        print(f"偵測到 {name}")
        print(distances[best_idx])

    cv2.imshow(filename, image)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

client.loop_stop()
client.disconnect()
