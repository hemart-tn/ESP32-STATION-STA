#include <Arduino.h>
#include <WiFi.h>
#include "LittleFS.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// ================= WIFI RUMAH =================
#define WIFI_SSID "HiFi-13897"
#define WIFI_PASS "pssslemanale"

// ================= PIN =================
#define trigPin 5
#define echoPin 18

#define RED_PIN   19
#define GREEN_PIN 21
#define BLUE_PIN  22

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

unsigned long previousMillis = 0;
const long interval = 100;

// ================= LITTLEFS =================
void initLittleFS() {
  if(!LittleFS.begin()){
    Serial.println("LittleFS Error");
    return;
  }
}

// ================= RGB =================
void setRGB(int r, int g, int b){
  ledcWrite(RED_PIN, r);
  ledcWrite(GREEN_PIN, g);
  ledcWrite(BLUE_PIN, b);
}

// ================= WEBSOCKET =================
void notifyClients(String msg) {
  ws.textAll(msg);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  data[len] = 0;
  String msg = String((char*)data);

  int r=0,g=0,b=0;

  if (msg.startsWith("RGB:")) {
    sscanf(msg.c_str(), "RGB:%d,%d,%d", &r,&g,&b);
    setRGB(r,g,b);
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len) {

  if(type == WS_EVT_DATA){
    handleWebSocketMessage(arg,data,len);
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

// ================= ULTRASONIC =================
int Read_Distance(){

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;

  return distance;
}

// ================= SETUP =================
void setup() {

  Serial.begin(115200);
  delay(1000);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  ledcAttach(RED_PIN, 5000, 8);
  ledcAttach(GREEN_PIN, 5000, 8);
  ledcAttach(BLUE_PIN, 5000, 8);

  initLittleFS();

  // ===== CONNECT WIFI =====
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  initWebSocket();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");
  server.begin();
}

// ================= LOOP =================
void loop() {

  if (millis() - previousMillis >= interval) {
    previousMillis = millis();

    int d = Read_Distance();
    notifyClients(String(d));
  }

  ws.cleanupClients();
}