#include <Arduino.h>
#include <header.h>

int loopTime;
Vector2 directionInput(0, 0);
float rotationInput = 0;
float groundClearance = 50;
float stepRadius = 100;
float maxSpeed = 90;
State HexapodState = State::SITTING;

WebServer server(80);
const char* ssid = "SpiderBot";
const char* password = "test1234";

const float movementSpeed = 500.0f;
const float rotationSpeed = 500.0f;

void setup() {
    Servo_init();
    directionInput = Vector2(0, 0);  // Explicit initialization
    rotationInput = 0;

    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    WiFi.softAP(ssid, password);
    Serial.print("IP Address: ");

    // Server endpoints unchanged
    server.on("/", HTTP_GET, []() { server.send(200, "text/html", htmlPage); });
    server.on("/led/on", HTTP_GET, []() { digitalWrite(ledPin, HIGH); server.send(200, "text/plain", "LED ON"); });
    server.on("/led/off", HTTP_GET, []() { digitalWrite(ledPin, LOW); server.send(200, "text/plain", "LED OFF"); });
    server.on("/move/forward", HTTP_GET, []() { directionInput = Vector2(movementSpeed, 0); rotationInput = 0; server.send(200, "text/plain", "Moving Forward"); });
    server.on("/move/backward", HTTP_GET, []() { directionInput = Vector2(-movementSpeed, 0); rotationInput = 0; server.send(200, "text/plain", "Moving Backward"); });
    server.on("/move/left", HTTP_GET, []() { directionInput = Vector2(0, -movementSpeed); rotationInput = 0; server.send(200, "text/plain", "Moving Left"); });
    server.on("/move/right", HTTP_GET, []() { directionInput = Vector2(0, movementSpeed); rotationInput = 0; server.send(200, "text/plain", "Moving Right"); });
    server.on("/rotate/ccw", HTTP_GET, []() { directionInput = Vector2(0, 0); rotationInput = -rotationSpeed; server.send(200, "text/plain", "Rotating CCW"); });
    server.on("/rotate/cw", HTTP_GET, []() { directionInput = Vector2(0, 0); rotationInput = rotationSpeed; server.send(200, "text/plain", "Rotating CW"); });
    server.on("/move/stop", HTTP_GET, []() { directionInput = Vector2(0, 0); rotationInput = 0; server.send(200, "text/plain", "Stopped"); });
    server.on("/sit", HTTP_GET, []() { sitDown(); server.send(200, "text/plain", "Sitting Down"); });
    server.on("/stand", HTTP_GET, []() { standUp(); server.send(200, "text/plain", "Standing Up"); });
    server.on("/ping", HTTP_GET, []() { String clientTime = server.arg("time"); server.send(200, "text/plain", clientTime); });

    server.begin();
    standUp();
}

unsigned long lastTime = 0;

void loop() {
    server.handleClient();

    unsigned long curTime = millis();
    loopTime = curTime - lastTime;
    lastTime = curTime;

    if (HexapodState != State::SITTING) {
        walkCycle();
        Output_update();
    }
}