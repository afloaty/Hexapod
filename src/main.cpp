#include <Arduino.h>
#include <header.h>

int loopTime;

Vector2 directionInput(0, 0);
float rotationInput = 20;
float groundClearance = 35;
float stepRadius = 100;
float maxSpeed = 90;

State HexapodState = State::SITTING;

WebServer server(80);
const char* ssid = "SpiderBot";
const char* password = "test1234";

void setup() {
  Servo_init(); // initialize Servos

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
      server.send(200, "text/plain", "Moving Forward");
  });
  server.on("/move/backward", HTTP_GET, []() {
      server.send(200, "text/plain", "Moving Backward");
  });
  server.on("/move/left", HTTP_GET, []() {
      server.send(200, "text/plain", "Turning Left");
  });
  server.on("/move/right", HTTP_GET, []() {
      server.send(200, "text/plain", "Turning Right");
  });
  server.on("/move/stop", HTTP_GET, []() {
      server.send(200, "text/plain", "Stopped");
  });

  server.on("/rotate/ccw", HTTP_GET, []() {
      server.send(200, "text/plain", "Rotating CCW");
  });
  server.on("/rotate/cw", HTTP_GET, []() {
      server.send(200, "text/plain", "Rotating CW");
  });

  server.on("/ping", HTTP_GET, []() {
      String clientTime = server.arg("time");
      server.send(200, "text/plain", clientTime);
  });

  server.begin();
  Serial.println("Web server started");

  #ifdef SERVO
  #ifdef BLUETOOTH
  Bluetooth_clear();
  Data[0] = 50;

  while (Data[0] == 50) // is 50 when it should sit down and 100 when it should stand up
  {
    Bluetooth_read();
    delay(10);
  }

  standUp();
#endif
#endif

#ifdef SERVO_CALIBRATION
  HexapodState = State::STANDING;
#endif
}

void loop() {
  server.handleClient();

  /*
  #ifdef SERVO

  // stand up and sit down as input suggests
  if (Data[0] == 50 && (HexapodState == State::STANDING || HexapodState == State::WALKING))
  {
    sitDown();
    curTime = millis(); // the sitDown function takes time so loopTime gets messed up
  }
  else if (Data[0] == 100 && HexapodState == State::SITTING)
  {
    standUp();
    curTime = millis(); // the sitDown function takes time so loopTime gets messed up
  }

  if (HexapodState == State::STANDING || HexapodState == State::WALKING)
  {
    walkCycle();
    Output_update();
  }
  #endif
  */
}

