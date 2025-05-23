import os
import pickle
import face_recognition

KNOWN_DIR = "train"
CACHE_PATH = "known_faces_cache.pkl"

known_encodings = []
known_names = []

for person_name in os.listdir(KNOWN_DIR):
    person_folder = os.path.join(KNOWN_DIR, person_name)
    if not os.path.isdir(person_folder):
        continue

    for filename in os.listdir(person_folder):
        img_path = os.path.join(person_folder, filename)
        img = face_recognition.load_image_file(img_path)
        encs = face_recognition.face_encodings(img)
        if encs:
            known_encodings.append(encs[0])
            known_names.append(person_name)

# 把 encodings 與 names 一起 dump 出去
with open(CACHE_PATH, "wb") as f:
    pickle.dump({
        "encodings": known_encodings,
        "names": known_names
    }, f)

print(f"已儲存 {len(known_names)} 個人臉編碼到 {CACHE_PATH}")
