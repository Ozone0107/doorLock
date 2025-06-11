#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include "ESP_I2S.h"
#include "BluetoothA2DPSink.h"
#include "esp_wifi.h"
#include <SPI.h>
#include <MFRC522.h>

// For esp wroom 32
#define STBY_PIN    13  // D13
#define AIN1_PIN    14  // D14
#define AIN2_PIN    32  // D27
#define PWMA_PIN    33  // D26

#define BUTTON_PIN 12

#define PIN_SCK     18
#define PIN_MOSI    23
#define PIN_MISO    19
#define PIN_SS      5
#define PIN_RST     15

const uint8_t I2S_SCK = 27;    /* Audio data bit clock */
const uint8_t I2S_WS = 26;     /* Audio data left and right clock */
const uint8_t I2S_SDOUT = 25;  /* ESP32 audio data output (to speakers) */
I2SClass i2s;

BluetoothA2DPSink a2dp_sink(i2s);

// WiFi
const char *ssid = "makerspace-2.4G"; // Enter your Wi-Fi name
const char *password = "ntueemakerspace";  // Enter Wi-Fi password

// MQTT Broker
const char *mqtt_broker = "test.mosquitto.org";
const char *topic = "face/motor/control";
const char *button_topic = "face/button/press";
const int mqtt_port = 1883;

bool isLocked = 1;
String uidStr;
String result;
String fullid;

MFRC522 mfrc522(PIN_SS, PIN_RST);
WiFiClient espClient;
PubSubClient client(espClient);

// FreeRTOS Queue for MQTT messages
QueueHandle_t mqttQueue;

// Structure to hold MQTT message data
typedef struct {
    char topic[64];
    char payload[64];
} MqttMessage_t;

byte allowedUIDs[][4] = {
    {0x45, 0x8E, 0x15, 0x52},
    {0x12, 0x34, 0x56, 0x78}
};

const int allowedCount = sizeof(allowedUIDs) / 4;

String formatUID(byte *uid, byte len) {
    String uidStr = "";
    for (byte i = 0; i < len; i++) {
        if (uid[i] < 0x10) uidStr += "0";
        uidStr += String(uid[i], HEX);
        if (i < len - 1) uidStr += " ";
    }
    return uidStr;
}

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

// FreeRTOS Task for MQTT Publishing
void MQTTPublishTask(void *pvParameters) {
    MqttMessage_t msg;
    while (true) {
        // 等待從 Queue 接收訊息
        if (xQueueReceive(mqttQueue, &msg, portMAX_DELAY) == pdPASS) {
            Serial.printf("MQTT Task: Publishing topic='%s', payload='%s'\n", msg.topic, msg.payload);
            if (client.connected()) {
                client.publish(msg.topic, msg.payload);
            } else {
                Serial.println("MQTT Task: Client not connected, message not published.");
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_SS);
    delay(100);
    mfrc522.PCD_Init();  // 初始化 RFID

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the Wi-Fi network");
    
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str())) {
            Serial.println("已連上公有 MQTT Broker");
        } else {
            Serial.printf("連線失敗, state=%d，3 秒後重試\n", client.state());
            delay(3000);
        }
    }
    client.subscribe(topic);
    // client.subscribe(button_topic); // MQTT publish task will handle button topic

    pinMode(AIN1_PIN, OUTPUT);
    pinMode(AIN2_PIN, OUTPUT);
    pinMode(PWMA_PIN, OUTPUT);
    pinMode(STBY_PIN, OUTPUT);
    digitalWrite(AIN1_PIN, LOW);
    digitalWrite(AIN2_PIN, LOW);
    analogWrite(PWMA_PIN, 0);
    digitalWrite(STBY_PIN, HIGH);
    
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    i2s.setPins(I2S_SCK, I2S_WS, I2S_SDOUT);
    if (!i2s.begin(I2S_MODE_STD, 44100, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH)) {
        Serial.println("Failed to initialize I2S!");
        while (1); // do nothing
    }

    a2dp_sink.start("MySpeaker");

    esp_wifi_set_ps(WIFI_PS_NONE);

    // Create FreeRTOS Queue for MQTT messages
    mqttQueue = xQueueCreate(10, sizeof(MqttMessage_t)); // Queue can hold 10 messages

    // Create FreeRTOS Task for MQTT Publishing
    xTaskCreate(
        MQTTPublishTask,      // Task function
        "MQTTPublishTask",    // Name of task
        4096,                 // Stack size (bytes)
        NULL,                 // Parameter of the task
        1,                    // Priority of the task (0 is the lowest)
        NULL                  // Task handle
    );
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    Serial.print((char) payload[0]);
    Serial.println();
    Serial.println("-----------------------");

    if((char) payload[0]=='1'){
        unlock();
        delay(7000);
        lock();
        delay(1000);
    }
}

void loop() {
    client.loop(); // Keep MQTT client connected and process incoming messages

    // Button handling
    if(digitalRead(BUTTON_PIN) == LOW){
        Serial.println("Button Pressed!");
        MqttMessage_t msg;
        strcpy(msg.topic, button_topic);
        strcpy(msg.payload, "p");
        // Send message to MQTT task via queue
        if (xQueueSend(mqttQueue, &msg, 0) != pdPASS) {
            Serial.println("Failed to send MQTT message to queue.");
        }
        Serial.println("Added 'p' to MQTT publish queue.");
        delay(1000); // Debounce delay for button
    }
    
    // RFID handling
    if (!mfrc522.PICC_IsNewCardPresent()) {
        delay(100);
        return;
    }
    Serial.println("✔ Card present");

    if (!mfrc522.PICC_ReadCardSerial()) {
        Serial.println("✖ Read card serial failed");
        delay(100);
        return;
    }
    Serial.println("✔ Read serial OK");
    uidStr = formatUID(mfrc522.uid.uidByte, mfrc522.uid.size);
    result = isUIDAllowed(mfrc522.uid.uidByte) ? "ALLOW" : "DENY";

    if(result == "ALLOW"){
        unlock();
        delay(7000);
        lock();
        delay(1000);
        result == "DENY"; // Reset result to prevent continuous unlocking
    }
    mfrc522.PICC_HaltA();
    delay(2000);
}