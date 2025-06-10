#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include "ESP_I2S.h"
#include "BluetoothA2DPSink.h"



// 這組腳位很屌，可以work，換之前再想一下(for esp32cam)
// #define AIN1_PIN    2   // 方向腳 1
// #define AIN2_PIN   13   // 方向腳 2
// #define PWMA_PIN   14   // PWM 腳
// #define STBY_PIN   0

// for esp wroom 32
#define STBY_PIN   13  // D13
#define AIN1_PIN   14  // D14
#define AIN2_PIN   32  // D27
#define PWMA_PIN   33  // D26

#define BUTTON_PIN 12

const uint8_t I2S_SCK = 27;       /* Audio data bit clock */
const uint8_t I2S_WS = 26;       /* Audio data left and right clock */
const uint8_t I2S_SDOUT = 25;    /* ESP32 audio data output (to speakers) */
I2SClass i2s;


BluetoothA2DPSink a2dp_sink(i2s);

// WiFi
// const char *ssid = "Ryan"; // Enter your Wi-Fi name
// const char *password = "ryanryan";  // Enter Wi-Fi password

// const char *ssid = "ooozone"; // Enter your Wi-Fi name
// const char *password = "aa12345269";  // Enter Wi-Fi password

const char *ssid = "makerspace-2.4G"; // Enter your Wi-Fi name
const char *password = "ntueemakerspace";  // Enter Wi-Fi password

// MQTT Broker
const char *mqtt_broker = "test.mosquitto.org";
const char *topic = "face/motor/control";
// const char *mqtt_username = "emqx";
// const char *mqtt_password = "public";
const int mqtt_port = 1883;

bool isLocked = 1;

WiFiClient espClient;
PubSubClient client(espClient);

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

    //pinMode(STBY_PIN, OUTPUT);
    //digitalWrite(STBY_PIN, LOW);
    // pinMode(AIN1_PIN, OUTPUT);
    // pinMode(AIN2_PIN, OUTPUT);
    // pinMode(PWMA_PIN, OUTPUT);
    //digitalWrite(STBY_PIN, HIGH);
    // Set software serial baud to 115200;

    i2s.setPins(I2S_SCK, I2S_WS, I2S_SDOUT);
    if (!i2s.begin(I2S_MODE_STD, 44100, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH)) {
      Serial.println("Failed to initialize I2S!");
      while (1); // do nothing
    }

    a2dp_sink.start("MySpeaker");

    Serial.begin(115200);
    // Connecting to a WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the Wi-Fi network");
    //connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        // if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
        //     Serial.println("Public EMQX MQTT broker connected");
        // } else {
        //     Serial.print("failed with state ");
        //     Serial.print(client.state());
        //     delay(2000);
        // }
        if (client.connect(client_id.c_str())) {
            Serial.println("已連上公有 MQTT Broker");
        } else {
            Serial.printf("連線失敗, state=%d，3 秒後重試\n", client.state());
            delay(3000);
        }
    }
    // Publish and subscribe
    //client.publish(topic, "Hi, I'm ESP32 ^^");
    client.subscribe(topic);

    pinMode(AIN1_PIN, OUTPUT);
    pinMode(AIN2_PIN, OUTPUT);
    pinMode(PWMA_PIN, OUTPUT);
    pinMode(STBY_PIN, OUTPUT);
    digitalWrite(AIN1_PIN, LOW);
    digitalWrite(AIN2_PIN, LOW);
    analogWrite(PWMA_PIN, 0);
    digitalWrite(STBY_PIN, HIGH);
  
    pinMode(BUTTON_PIN, INPUT_PULLUP);
  
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
    
    /*
    else if((char) payload[0]=='0'){ 
      digitalWrite(AIN1_PIN, LOW);
      digitalWrite(AIN2_PIN, LOW);
      analogWrite(PWMA_PIN, 0);
    }
    */
}

void loop() {
    client.loop();
    if(digitalRead(BUTTON_PIN) == LOW){
        unlock();
        delay(7000);
        lock();
        delay(1000);
    }
}





