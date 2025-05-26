#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>


// 這組腳位很屌，可以work，換之前再想一下
#define AIN1_PIN    2   // 方向腳 1
#define AIN2_PIN   13   // 方向腳 2
#define PWMA_PIN   14   // PWM 腳
#define STBY_PIN   0
// WiFi
const char *ssid = "Ryan"; // Enter your Wi-Fi name
const char *password = "ryanryan";  // Enter Wi-Fi password

// MQTT Broker
const char *mqtt_broker = "test.mosquitto.org";
const char *topic = "face/motor/control";
// const char *mqtt_username = "emqx";
// const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {

    //pinMode(STBY_PIN, OUTPUT);
    //digitalWrite(STBY_PIN, LOW);
    // pinMode(AIN1_PIN, OUTPUT);
    // pinMode(AIN2_PIN, OUTPUT);
    // pinMode(PWMA_PIN, OUTPUT);
    //digitalWrite(STBY_PIN, HIGH);
    // Set software serial baud to 115200;

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
  
}



void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    Serial.print((char) payload[0]);
    Serial.println();
    Serial.println("-----------------------");
    if((char) payload[0]=='1'){
      digitalWrite(AIN1_PIN, HIGH);
      digitalWrite(AIN2_PIN, LOW);
      analogWrite(PWMA_PIN, 200);  // 0–255 之間 (約 70%)
      delay(3000);
    }else if((char) payload[0]=='0'){
      digitalWrite(AIN1_PIN, LOW);
      digitalWrite(AIN2_PIN, LOW);
      analogWrite(PWMA_PIN, 0);
    }
}

void loop() {
    client.loop();
}