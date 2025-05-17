#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "esp_camera.h"
#include "esp_timer.h"

// ক্যামেরা কনফিগারেশন (AI-Thinker ESP32-CAM)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// মোটর পিন কনফিগারেশন
#define ENA 14  // বাম মোটর PWM
#define IN1 12  // বাম সামনে
#define IN2 13  // বাম পিছনে
#define IN3 15  // ডান সামনে
#define IN4 2   // ডান পিছনে
#define ENB 4   // ডান মোটর PWM

WebServer server(80);
int carSpeed = 180; // ডিফল্ট গতি (0-255)

void setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("ক্যামেরা ইন্সটলেশনে ব্যর্থ: 0x%x", err);
    ESP.restart();
  }
}

void setupMotors() {
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  
  ledcSetup(0, 5000, 8); // PWM চ্যানেল 0
  ledcSetup(1, 5000, 8); // PWM চ্যানেল 1
  ledcAttachPin(ENA, 0);
  ledcAttachPin(ENB, 1);
  
  stopCar();
}

void startCameraServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/stream", HTTP_GET, handleStream);
  server.on("/forward", HTTP_GET, []() { moveCar("forward"); server.send(200, "text/plain", "OK"); });
  server.on("/backward", HTTP_GET, []() { moveCar("backward"); server.send(200, "text/plain", "OK"); });
  server.on("/left", HTTP_GET, []() { moveCar("left"); server.send(200, "text/plain", "OK"); });
  server.on("/right", HTTP_GET, []() { moveCar("right"); server.send(200, "text/plain", "OK"); });
  server.on("/stop", HTTP_GET, []() { moveCar("stop"); server.send(200, "text/plain", "OK"); });
  server.on("/speed", HTTP_GET, handleSpeed);
  
  server.begin();
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>রোবট কার কন্ট্রোল</title>
  <style>
    body {
      font-family: 'Arial', sans-serif;
      text-align: center;
      background: #f5f5f5;
      margin: 0;
      padding: 20px;
    }
    .control-panel {
      background: white;
      border-radius: 10px;
      padding: 20px;
      max-width: 400px;
      margin: 0 auto;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    }
    h1 {
      color: #2c3e50;
      margin-bottom: 20px;
    }
    #stream {
      border: 2px solid #ddd;
      border-radius: 5px;
      margin-bottom: 15px;
      max-width: 100%;
    }
    .btn {
      font-size: 16px;
      padding: 12px 24px;
      margin: 8px;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      color: white;
      transition: background 0.3s;
    }
    .btn-forward { background: #27ae60; }
    .btn-backward { background: #2980b9; }
    .btn-left { background: #f39c12; }
    .btn-right { background: #e74c3c; }
    .btn-stop { background: #7f8c8d; }
    .btn:hover {
      opacity: 0.9;
    }
    .speed-control {
      margin: 20px 0;
    }
    input[type="range"] {
      width: 100%;
      margin: 10px 0;
    }
  </style>
</head>
<body>
  <div class="control-panel">
    <h1>রোবট কার কন্ট্রোল</h1>
    <img id="stream" src="/stream" width="320" height="240">
    
    <div class="speed-control">
      <label for="speedSlider">গতি: <span id="speedValue">180</span></label>
      <input type="range" id="speedSlider" min="0" max="255" value="180" oninput="updateSpeed(this.value)">
    </div>
    
    <button class="btn btn-forward" ontouchstart="sendCmd('forward')" ontouchend="sendCmd('stop')" 
            onmousedown="sendCmd('forward')" onmouseup="sendCmd('stop')">সামনে</button><br>
    <button class="btn btn-left" ontouchstart="sendCmd('left')" ontouchend="sendCmd('stop')" 
            onmousedown="sendCmd('left')" onmouseup="sendCmd('stop')">বামে</button>
    <button class="btn btn-stop" onclick="sendCmd('stop')">থামুন</button>
    <button class="btn btn-right" ontouchstart="sendCmd('right')" ontouchend="sendCmd('stop')" 
            onmousedown="sendCmd('right')" onmouseup="sendCmd('stop')">ডানে</button><br>
    <button class="btn btn-backward" ontouchstart="sendCmd('backward')" ontouchend="sendCmd('stop')" 
            onmousedown="sendCmd('backward')" onmouseup="sendCmd('stop')">পিছনে</button>
  </div>

  <script>
    function sendCmd(cmd) {
      fetch('/' + cmd).catch(err => console.error('Error:', err));
    }
    function updateSpeed(val) {
      document.getElementById('speedValue').textContent = val;
      fetch('/speed?value=' + val).catch(err => console.error('Error:', err));
    }
  </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

void handleStream() {
  WiFiClient client = server.client();
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);

  while (client.connected()) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("ক্যামেরা ফ্রেম ক্যাপচার ব্যর্থ");
      continue;
    }

    server.sendContent("--frame\r\n");
    server.sendContent("Content-Type: image/jpeg\r\n\r\n");
    server.sendContent((const char*)fb->buf, fb->len);
    server.sendContent("\r\n");
    esp_camera_fb_return(fb);
    delay(30);
  }
}

void handleSpeed() {
  if (server.hasArg("value")) {
    carSpeed = server.arg("value").toInt();
    Serial.printf("নতুন গতি সেট করা হয়েছে: %d\n", carSpeed);
  }
  server.send(200, "text/plain", "OK");
}

void moveCar(String direction) {
  if (direction == "forward") {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  } 
  else if (direction == "backward") {
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  } 
  else if (direction == "left") {
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  } 
  else if (direction == "right") {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  } 
  else { // stop
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  }
  
  ledcWrite(0, carSpeed); // বাম মোটর
  ledcWrite(1, carSpeed); // ডান মোটর
}

void stopCar() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  ledcWrite(0, 0);
  ledcWrite(1, 0);
}

void setup() {
  Serial.begin(115200);
  
  // ক্যামেরা সেটআপ
  setupCamera();
  
  // মোটর সেটআপ
  setupMotors();
  
  // Wi-Fi হটস্পট শুরু করুন
  WiFi.softAP("RobotCar_CAM", "12345678");
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());
  
  // ক্যামেরা সার্ভার শুরু করুন
  startCameraServer();
}

void loop() {
  server.handleClient();
  delay(2);
}
