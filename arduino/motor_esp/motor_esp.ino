#include <Arduino.h>


// 這組腳位很屌，可以work，換之前再想一下
// #define AIN1_PIN    2   // 方向腳 1
// #define AIN2_PIN   13   // 方向腳 2
// #define PWMA_PIN   14   // PWM 腳
// #define STBY_PIN   0

// #define STBY_PIN   13  // D13
// #define AIN1_PIN   14  // D14
// #define AIN2_PIN   27  // D27
// #define PWMA_PIN   26  // D26

// #define BUTTON_PIN 25

//works

#define STBY_PIN   13  // D13
#define AIN1_PIN   14  // D14
#define AIN2_PIN   32  // D27
#define PWMA_PIN   33  // D26

#define BUTTON_PIN 12

void unlock() {
    digitalWrite(AIN1_PIN, HIGH);
    digitalWrite(AIN2_PIN, LOW);
    analogWrite(PWMA_PIN, 220);  // 0–255 之間 (約 70%)
    delay(170);
    analogWrite(PWMA_PIN, 0);   
}

void lock() {
    digitalWrite(AIN1_PIN, LOW);
    digitalWrite(AIN2_PIN, HIGH);
    analogWrite(PWMA_PIN, 220);  // 0–255 之間 (約 70%)
    delay(170);
    analogWrite(PWMA_PIN, 0);   
}

void setup() {
  // 初始化腳位
    pinMode(AIN1_PIN, OUTPUT);
    pinMode(AIN2_PIN, OUTPUT);
    pinMode(PWMA_PIN, OUTPUT);
    pinMode(STBY_PIN, OUTPUT);
    digitalWrite(AIN1_PIN, LOW);
    digitalWrite(AIN2_PIN, LOW);
    analogWrite(PWMA_PIN, 0);
    digitalWrite(STBY_PIN, HIGH);
  
    pinMode(BUTTON_PIN, INPUT_PULLUP);

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
  //範例：馬達正轉 70% 速度，持續 2 秒
if (digitalRead(BUTTON_PIN) == LOW){
  // 開鎖
  lock();
  delay(1000);
};


//delay(1000);


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
