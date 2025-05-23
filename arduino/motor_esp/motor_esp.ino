#include <Arduino.h>

// TB6612FNG A 通道腳位
//#define STBY_PIN   13   // 使能腳 (Standby)，拉高啟用
#define AIN1_PIN    2   // 方向腳 1
#define AIN2_PIN   15   // 方向腳 2
#define PWMA_PIN   16   // PWM 腳
#define STBY_PIN   1

void setup() {
  // 初始化腳位
  pinMode(STBY_PIN, OUTPUT);
  digitalWrite(STBY_PIN, LOW);
  pinMode(AIN1_PIN, OUTPUT);
  pinMode(AIN2_PIN, OUTPUT);
  pinMode(PWMA_PIN, OUTPUT);

  //使能 TB6612，啟用 A/B 通道輸出
//   digitalWrite(STBY_PIN, LOW);

// //馬達預設停轉
  // digitalWrite(AIN1_PIN, LOW);
  // digitalWrite(AIN2_PIN, LOW);
  // analogWrite(PWMA_PIN, 0);

  //delay(1000);


  //digitalWrite(STBY_PIN, HIGH);

//   delay(500);
 }

void loop() {
  // 範例：馬達正轉 70% 速度，持續 2 秒
  // digitalWrite(AIN1_PIN, HIGH);
  // digitalWrite(AIN2_PIN, LOW);
  // analogWrite(PWMA_PIN, 200);  // 0–255 之間 (約 70%)
  // delay(3000);

  // // 範例：馬達反轉 50% 速度，持續 2 秒
  // digitalWrite(AIN1_PIN, LOW);
  // digitalWrite(AIN2_PIN, HIGH);
  // analogWrite(PWMA_PIN, 128);  // 約 50%
  // delay(2000);

  // 範例：停止 1 秒
  // analogWrite(PWMA_PIN, 0);
  // delay(1000);
}
