#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi 設定
// const char* ssid = "Ryan";
// const char* password = "ryanryan";

const char* ssid = "ooozone";
const char* password = "aa12345269";

// MQTT 設定
const char* mqtt_server = "test.mosquitto.org";
const char* topic_uid = "rfid/uid";
const char* topic_status = "rfid/status";

// MFRC522 接腳
#define PIN_SCK   13
#define PIN_MOSI  15
#define PIN_MISO  14
#define PIN_SS    12
#define PIN_RST   0


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
  Serial.begin(115200);
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_SS);
  //SPI.begin();         // 預設 SPI 腳位 (18, 19, 23)
  delay(100);
  mfrc522.PCD_Init();  // 初始化 RFID
  connectWiFi();

  client.setServer(mqtt_server, 1883);
  reconnectMQTT();
  client.publish(topic_status, "start!!!!!");
  Serial.println("Ready to scan RFID cards");
}

void loop() {

  mfrc522.PCD_Init();  // 初始化 RFID
  if (!client.connected()) reconnectMQTT();
  client.loop();

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) return;

  byte *uid = mfrc522.uid.uidByte;
  byte uidSize = mfrc522.uid.size;
  String uidStr = formatUID(uid, uidSize);

  Serial.print("UID: "); Serial.println(uidStr);
  client.publish(topic_uid, uidStr.c_str());

  String result = isUIDAllowed(uid) ? "ALLOW" : "DENY";
  client.publish(topic_status, result.c_str());
  Serial.println("Status: " + result);

  mfrc522.PICC_HaltA();  // 停止卡片通訊
  delay(2000);

}