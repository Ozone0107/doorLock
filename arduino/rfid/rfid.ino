#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi 設定
// const char* ssid = "Ryan";
// const char* password = "ryanryan";

// const char* ssid = "makerspace-2.4G";
// const char* password = "ntueemakerspace";

const char* ssid = "ooozone";
const char* password = "aa12345269";


// MQTT 設定
const char* mqtt_server = "test.mosquitto.org";
const char* topic_uid = "rfid/uid";
const char* topic_status = "rfid/status";

// MFRC522 接腳
// #define PIN_SCK   18
// #define PIN_MOSI  23
// #define PIN_MISO  19
// #define PIN_SS    5
// #define PIN_RST   15

#define PIN_MISO        12
#define PIN_MOSI        13
#define PIN_SCK         14
#define PIN_SS          15
#define PIN_RST         16


MFRC522 mfrc522(PIN_SS, PIN_RST);
WiFiClient espClient;
PubSubClient client(espClient);

// 白名單 UID（4-byte 範例）
byte allowedUIDs[][4] = {
  {0xDE, 0xAD, 0xBE, 0xEF},
  {0x12, 0x34, 0x56, 0x78}
};
const int allowedCount = sizeof(allowedUIDs) / 4;

// 函數：格式化 UID 為 HEX 字串
String formatUID(byte *uid, byte len) {
  String uidStr = "";
  for (byte i = 0; i < len; i++) {
    if (uid[i] < 0x10) uidStr += "0";
    uidStr += String(uid[i], HEX);
    if (i < len - 1) uidStr += " ";
  }
  return uidStr;
}

// 函數：檢查 UID 是否在白名單中
bool isUIDAllowed(byte *uid) {
  for (int i = 0; i < allowedCount; i++) {
    bool match = true;
    for (int j = 0; j < 4; j++) {
      if (uid[j] != allowedUIDs[i][j]) {
        match = false;
        break;
      }
    }
    if (match) return true;
  }
  return false;
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("WiFi connected");
}

void reconnectMQTT() {
  while (!client.connected()) {
    if (client.connect("ESP32_RFID")) {
      Serial.println("Connected to MQTT broker");
    } else {
      delay(1000);
    }
  }
}

void setup() {
  delay(2000);
  Serial.begin(115200);
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_SS);
  //SPI.begin();         // 預設 SPI 腳位 (18, 19, 23)

  mfrc522.PCD_Init();  // 初始化 RFID
  delay(100);
  connectWiFi();

  client.setServer(mqtt_server, 1883);
  reconnectMQTT();
  client.publish(topic_status, "start!!!!!");
  Serial.println("Ready to scan RFID cards");
}

void loop() {
  client.loop();

  // 调试：每秒打印一次心跳
  static unsigned long last = 0;
  if (millis() - last > 1000) {
    last = millis();
    //Serial.println("⏱ loop heartbeat");
  }

  // 1) 检测新卡
  if (!mfrc522.PICC_IsNewCardPresent()) {
    //Serial.println("…no card…");
    delay(100);
    return;
  }
  Serial.println("✔ Card present");
 

  // 2) 读序列号
  if (!mfrc522.PICC_ReadCardSerial()) {
    Serial.println("✖ Read card serial failed");
    delay(100);
    return;
  }
  Serial.println("✔ Read serial OK");

  // 3) 格式化 UID
  String uidStr = formatUID(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println("📀 UID: " + uidStr);

  // 4) 发布到 MQTT
  client.publish(topic_uid, uidStr.c_str());
  Serial.println("📡 Published UID");

  String result = isUIDAllowed(mfrc522.uid.uidByte) ? "ALLOW" : "DENY";
  client.publish(topic_status, result.c_str());
  Serial.println("🔔 Published status: " + result);

  mfrc522.PICC_HaltA();
  delay(2000);
}