#include <Arduino.h>
#include <header.h>

int loopTime;
Vector2 directionInput(0, 0);
float rotationInput = 0;
float groundClearance = 35;
float stepRadius = 100;
float maxSpeed = 90;
State HexapodState = State::SITTING;

WebServer server(80);
const char* ssid = "SpiderBot";
const char* password = "test1234";

const float movementSpeed = 120.0f;  // mm/s
const float rotationSpeed = 90.0f;  // degrees per second

void setup() {
  Servo_init(); // Initialize servos

  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  WiFi.softAP(ssid, password);
  Serial.println("WiFi AP started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, []() {
      server.send(200, "text/html", htmlPage);
  });

  server.on("/led/on", HTTP_GET, []() {
      digitalWrite(ledPin, HIGH);
      Serial.println("LED ON");
      server.send(200, "text/plain", "LED ON");
  });

  server.on("/led/off", HTTP_GET, []() {
      digitalWrite(ledPin, LOW);
      Serial.println("LED OFF");
      server.send(200, "text/plain", "LED OFF");
  });

  server.on("/move/forward", HTTP_GET, []() {
      directionInput = Vector2(movementSpeed, 0);
      rotationInput = 0;
      Serial.println("Moving Forward");
      server.send(200, "text/plain", "Moving Forward");
  });

  server.on("/move/backward", HTTP_GET, []() {
      directionInput = Vector2(-movementSpeed, 0);
      rotationInput = 0;
      Serial.println("Moving Backward");
      server.send(200, "text/plain", "Moving Backward");
  });

  server.on("/move/left", HTTP_GET, []() {
      directionInput = Vector2(0, -movementSpeed);
      rotationInput = 0;
      Serial.println("Moving Left");
      server.send(200, "text/plain", "Moving Left");
  });

  server.on("/move/right", HTTP_GET, []() {
      directionInput = Vector2(0, movementSpeed);
      rotationInput = 0;
      Serial.println("Moving Right");
      server.send(200, "text/plain", "Moving Right");
  });

  server.on("/rotate/ccw", HTTP_GET, []() {
      directionInput = Vector2(0, 0);
      rotationInput = -rotationSpeed;
      Serial.println("Rotating CCW");
      server.send(200, "text/plain", "Rotating CCW");
  });

  server.on("/rotate/cw", HTTP_GET, []() {
      directionInput = Vector2(0, 0);
      rotationInput = rotationSpeed;
      Serial.println("Rotating CW");
      server.send(200, "text/plain", "Rotating CW");
  });

  server.on("/move/stop", HTTP_GET, []() {
      directionInput = Vector2(0, 0);
      rotationInput = 0;
      Serial.println("Stopped");
      server.send(200, "text/plain", "Stopped");
  });

  server.on("/ping", HTTP_GET, []() {
      String clientTime = server.arg("time");
      server.send(200, "text/plain", clientTime);
  });

  server.begin();
  Serial.println("Web server started");

  standUp();  // Stand up at startup
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
