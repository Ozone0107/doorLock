#include "esp_camera.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>        // ← 确保加上这行

// ——— 摄像头引脚（AI-Thinker） ———
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM       5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ——— Wi-Fi + MQTT ———
const char* ssid       = "Ryan";
const char* password   = "ryanryan";
const char* mqttBroker = "test.mosquitto.org";
const int   mqttPort   = 1883;
const char* mqttTopic  = "face/motor/control";

WiFiClient    wifiClient;
PubSubClient  mqtt(wifiClient);

// ——— 马达脚 ———
#define AIN1_PIN 2
#define AIN2_PIN 13
#define PWMA_PIN 14
#define STBY_PIN 0

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char cmd = (char)payload[0];
  if (cmd == '1') {
    digitalWrite(AIN1_PIN, HIGH);
    digitalWrite(AIN2_PIN, LOW);
    analogWrite(PWMA_PIN, 200);
  } else {
    digitalWrite(AIN1_PIN, LOW);
    digitalWrite(AIN2_PIN, LOW);
    analogWrite(PWMA_PIN, 0);
  }
}

void setup() {
  Serial.begin(115200);

  // 马达
  pinMode(AIN1_PIN, OUTPUT);
  pinMode(AIN2_PIN, OUTPUT);
  pinMode(PWMA_PIN, OUTPUT);
  pinMode(STBY_PIN, OUTPUT);
  digitalWrite(STBY_PIN, HIGH);

  // Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWi-Fi OK: " + WiFi.localIP().toString());

  // 摄像头配置，逐个赋值
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_QVGA;
  config.fb_location  = CAMERA_FB_IN_DRAM;
  config.fb_count     = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    while(true) delay(1000);
  }
  Serial.println("Camera init OK");

  // MQTT
  mqtt.setServer(mqttBroker, mqttPort);
  mqtt.setCallback(mqttCallback);
  while (!mqtt.connected()) {
    if (mqtt.connect("esp32cam-client")) {
      mqtt.subscribe(mqttTopic);
    } else {
      delay(3000);
    }
  }
}

void loop() {
  mqtt.loop();

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Capture failed");
    delay(1000);
    return;
  }

  HTTPClient http;
  http.begin("http://192.168.50.164:5000/recognize");
  http.addHeader("Content-Type", "multipart/form-data; boundary=ESP32CAM");
  String boundary = "ESP32CAM";
  String head = "--" + boundary + "\r\n"
                "Content-Disposition: form-data; name=\"image\"; filename=\"img.jpg\"\r\n"
                "Content-Type: image/jpeg\r\n\r\n";
  String tail = "\r\n--" + boundary + "--\r\n";
  int total = head.length() + fb->len + tail.length();
  http.addHeader("Content-Length", String(total));

  WiFiClient* stream = http.getStreamPtr();
  stream->print(head);
  stream->write(fb->buf, fb->len);
  stream->print(tail);

  int code = http.POST("");
  String resp = http.getString();
  Serial.printf("HTTP %d %s\n", code, resp.c_str());

  http.end();
  esp_camera_fb_return(fb);

  delay(5000);
}
