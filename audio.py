import pygame
import time

# 初始化 Pygame 混音器
pygame.mixer.init()

# 載入音訊檔案
# pygame.mixer.Sound() 適合播放短音效
# pygame.mixer.music.load() 適合播放長音樂 (如背景音樂)
# 這裡我們用 Sound 來保持和之前 playsound 的行為一致
sound = pygame.mixer.Sound('familymart.wav')

print("開始播放音訊 (非阻塞)...")
sound.play() # 預設就是非阻塞的，會立即返回

print("音訊播放命令已發出，程式繼續執行其他任務。")

# 這裡你可以執行其他程式碼，音訊會在背景播放
for i in range(5):
    print(f"主程式正在執行任務 {i+1}/5...")
    time.sleep(1) # 模擬其他任務的耗時

# 你可以檢查音訊是否還在播放
if pygame.mixer.get_busy():
    print("音訊可能仍在播放中。")
else:
    print("音訊已播放完成。")

# 如果你的程式是短命的，且需要確保音效播放完畢才退出，
# 你可能需要像之前的範例一樣等待：
# while pygame.mixer.get_busy():
#     time.sleep(0.1) # 短暫等待，讓主執行緒不完全卡死

# 清理 Pygame 資源
pygame.mixer.quit()
print("Pygame 混音器已關閉。")