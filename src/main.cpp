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
const float rotationSpeed = movementSpeed;
float speedMultiplier = 1.0f;  // Default speed multiplier

void setup() {
    Wire.begin();           // Initialize I2C bus
    delay(10);             // Small delay to stabilize the bus

    Servo_init();          // Initialize PCA9685 drivers and servo configurations
    setAllServosToNeutral(); // Set all servos to a neutral position
    delay(500);            // Allow time for servos to stabilize

    directionInput = Vector2(0, 0);
    rotationInput = 0;

    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    WiFi.softAP(ssid, password);
    Serial.print("IP Address: ");

    server.on("/", HTTP_GET, []() { server.send(200, "text/html", htmlPage); });
    server.on("/led/on", HTTP_GET, []() { digitalWrite(ledPin, HIGH); server.send(200, "text/plain", "LED ON"); });
    server.on("/led/off", HTTP_GET, []() { digitalWrite(ledPin, LOW); server.send(200, "text/plain", "LED OFF"); });
    // Add new server endpoints for speed control
    server.on("/speed/increase", HTTP_GET, []() {
        speedMultiplier = constrain(speedMultiplier + 0.1f, 0.0f, 2.0f);
        server.send(200, "text/plain", "Speed: " + String(speedMultiplier));
    });
    server.on("/speed/decrease", HTTP_GET, []() {
        speedMultiplier = constrain(speedMultiplier - 0.1f, 0.0f, 2.0f);
        server.send(200, "text/plain", "Speed: " + String(speedMultiplier));
    });
    server.on("/speed/reset", HTTP_GET, []() {
        speedMultiplier = 1.0f;
        server.send(200, "text/plain", "Speed: " + String(speedMultiplier));
    });
    // Modify existing movement endpoints to use speedMultiplier
    server.on("/move/forward", HTTP_GET, []() { 
        directionInput = Vector2(movementSpeed * speedMultiplier, 0); 
        rotationInput = 0; 
        server.send(200, "text/plain", "Moving Forward"); 
    });
    server.on("/move/backward", HTTP_GET, []() { 
        directionInput = Vector2(-movementSpeed * speedMultiplier, 0); 
        rotationInput = 0; 
        server.send(200, "text/plain", "Moving Backward"); 
    });
    server.on("/move/left", HTTP_GET, []() { 
        directionInput = Vector2(0, movementSpeed * speedMultiplier); 
        rotationInput = 0; 
        server.send(200, "text/plain", "Moving Left"); 
    });
    server.on("/move/right", HTTP_GET, []() { 
        directionInput = Vector2(0, -movementSpeed * speedMultiplier); 
        rotationInput = 0; 
        server.send(200, "text/plain", "Moving Right"); 
    });
    server.on("/rotate/ccw", HTTP_GET, []() { 
        directionInput = Vector2(0, 0); 
        rotationInput = -rotationSpeed * speedMultiplier; 
        server.send(200, "text/plain", "Rotating CCW"); 
    });
    server.on("/rotate/cw", HTTP_GET, []() { 
        directionInput = Vector2(0, 0); 
        rotationInput = rotationSpeed * speedMultiplier; 
        server.send(200, "text/plain", "Rotating CW"); 
    });
    server.on("/move/stop", HTTP_GET, []() { directionInput = Vector2(0, 0); rotationInput = 0; server.send(200, "text/plain", "Stopped"); });
    server.on("/sit", HTTP_GET, []() { sitDown(); server.send(200, "text/plain", "Sitting Down"); });
    server.on("/stand", HTTP_GET, []() { standUp(); server.send(200, "text/plain", "Standing Up"); });
    server.on("/ping", HTTP_GET, []() { String clientTime = server.arg("time"); server.send(200, "text/plain", clientTime); });
    
// New endpoints for speed control
    server.on("/speed/set", HTTP_GET, []() {
        if (server.hasArg("value")) {
          speedMultiplier = server.arg("value").toFloat();
          speedMultiplier = constrain(speedMultiplier, 0.0, 2.0);
          server.send(200, "text/plain", String(speedMultiplier));
        } else {
          server.send(400, "text/plain", "Missing value parameter");
        }
      });
    server.on("/speed/get", HTTP_GET, []() {
        server.send(200, "text/plain", String(speedMultiplier));
      });
    
      // New endpoints for ground clearance
    server.on("/groundClearance/set", HTTP_GET, []() {
        if (server.hasArg("value")) {
          groundClearance = server.arg("value").toFloat();
          groundClearance = constrain(groundClearance, 0.0, 100.0);
          server.send(200, "text/plain", String(groundClearance));
        } else {
          server.send(400, "text/plain", "Missing value parameter");
        }
      });
    server.on("/groundClearance/get", HTTP_GET, []() {
        server.send(200, "text/plain", String(groundClearance));
      });
    
      // New endpoints for step radius
    server.on("/stepRadius/set", HTTP_GET, []() {
        if (server.hasArg("value")) {
          stepRadius = server.arg("value").toFloat();
          stepRadius = constrain(stepRadius, 0.0, 200.0);
          server.send(200, "text/plain", String(stepRadius));
        } else {
          server.send(400, "text/plain", "Missing value parameter");
        }
      });
    server.on("/stepRadius/get", HTTP_GET, []() {
        server.send(200, "text/plain", String(stepRadius));
      });
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