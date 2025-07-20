#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <LittleFS.h>
#include "Config.h"

// Servo object
Servo testServo;

// Web server
AsyncWebServer server(80);

// Non-blocking servo test variables
bool servoTestActive = false;
int servoTestIndex = 0;
unsigned long servoTestLastMove = 0;
const unsigned long servoMoveDelay = 1000; // 1 second between moves
int testPositions[] = {0, 30, 60, 90, 120, 150, 180, 90, 0};
const int numTestPositions = sizeof(testPositions) / sizeof(testPositions[0]);

// Manual servo position control
int currentServoPosition = 90;

// Function declarations
void testServoMovement();
void updateServoTest();

void setupServo() {
  Serial.println("=== SERVO SETUP ===");
  Serial.printf("Attaching servo to pin %d...\n", servoPin);
  Serial.println("IMPORTANT: Ensure MG996R has external 5V power!");
  Serial.println("Wiring: Brown->GND, Red->5V External, Yellow->GPIO5");
  
  // Configure servo with specific pulse width range for MG996R
  // MG996R typically uses 1000-2000μs pulse width
  testServo.attach(servoPin, 1000, 2000);  // min=1000μs, max=2000μs
  
  Serial.println("Moving to rest position...");
  testServo.write(servoRestAngle);
  Serial.printf("Servo commanded to rest position: %d degrees\n", servoRestAngle);
  currentServoPosition = servoRestAngle;
  
  delay(1000); // Give servo more time to move
  Serial.println("Testing servo responsiveness...");
  
  // Quick test movement
  testServo.write(45);
  delay(500);
  testServo.write(servoRestAngle);
  delay(500);
  
  Serial.println("=== SERVO READY ===\n");
}

void setupWiFi() {
  Serial.println("Starting WiFi connection...");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi!");
    Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nWiFi connection failed!");
  }
}

void setupWebServer() {
  if (!LittleFS.begin(true)) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  // Servo test endpoint
  server.on("/servo/test", HTTP_POST, [](AsyncWebServerRequest *request){
    testServoMovement();
    request->send(200, "application/json", "{\"status\":\"servo test started\"}");
  });

  // Manual servo position control
  server.on("/servo/position", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("angle", true)) {
      int angle = request->getParam("angle", true)->value().toInt();
      if (angle >= 0 && angle <= 180) {
        Serial.printf("Web request: Moving servo from %d to %d degrees\n", currentServoPosition, angle);
        testServo.write(angle);
        currentServoPosition = angle;
        Serial.printf("Servo commanded to %d degrees\n", angle);
        request->send(200, "application/json", 
                     String("{\"status\":\"servo moved\",\"position\":") + angle + "}");
      } else {
        Serial.printf("Invalid angle requested: %d (must be 0-180)\n", angle);
        request->send(400, "application/json", "{\"error\":\"angle must be 0-180\"}");
      }
    } else {
      Serial.println("Missing angle parameter in servo position request");
      request->send(400, "application/json", "{\"error\":\"missing angle parameter\"}");
    }
  });

  // Get current servo position
  server.on("/servo/position", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", 
                 String("{\"position\":") + currentServoPosition + "}");
  });

  // Serve static files
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
  
  server.begin();
  Serial.println("Web server started!");
}

void testServoMovement() {
  if (!servoTestActive) {
    Serial.println("\n=== SERVO MOVEMENT TEST ===");
    Serial.println("Testing servo range and movement...");
    servoTestActive = true;
    servoTestIndex = 0;
    servoTestLastMove = millis();
    
    // Start with first position
    Serial.printf("Moving servo to %d degrees...\n", testPositions[0]);
    testServo.write(testPositions[0]);
    currentServoPosition = testPositions[0];
  }
}

void updateServoTest() {
  if (!servoTestActive) return;
  
  unsigned long currentTime = millis();
  
  // Check if it's time for the next servo movement
  if (currentTime - servoTestLastMove >= servoMoveDelay) {
    Serial.printf("Servo commanded to: %d degrees\n", testPositions[servoTestIndex]);
    
    servoTestIndex++;
    if (servoTestIndex < numTestPositions) {
      // Move to next position
      Serial.printf("Moving servo to %d degrees...\n", testPositions[servoTestIndex]);
      testServo.write(testPositions[servoTestIndex]);
      currentServoPosition = testPositions[servoTestIndex];
      servoTestLastMove = currentTime;
    } else {
      // Test complete
      testServo.write(servoRestAngle);
      currentServoPosition = servoRestAngle;
      Serial.printf("Servo returned to rest position: %d degrees\n", servoRestAngle);
      Serial.println("=== SERVO TEST COMPLETE ===\n");
      servoTestActive = false;
    }
  }
}

void printStatus() {
  static unsigned long lastStatusPrint = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastStatusPrint > 5000) { // Every 5 seconds
    Serial.println("\n--- SERVO STATUS ---");
    Serial.printf("Uptime: %lu seconds\n", currentTime / 1000);
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Temperature: %.1f°C\n", temperatureRead());
    Serial.printf("Servo Position: %d degrees\n", currentServoPosition);
    Serial.printf("Test Active: %s\n", servoTestActive ? "YES" : "NO");
    Serial.printf("WiFi: %s", WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf(" (RSSI: %d dBm)", WiFi.RSSI());
    }
    Serial.println();
    Serial.println("--- END STATUS ---\n");
    lastStatusPrint = currentTime;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n==================================================");
  Serial.println("         ESP32 SERVO TEST FIRMWARE");
  Serial.println("==================================================");
  Serial.printf("Chip Model: %s\n", ESP.getChipModel());
  Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.println("==================================================\n");
  
  setupServo();
  setupWiFi();
  setupWebServer();
  
  Serial.println("**************************************************");
  Serial.println("       SERVO TEST SYSTEM READY!");
  Serial.println("**************************************************");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Web interface: http://%s\n", WiFi.localIP().toString().c_str());
  }
  Serial.println("**************************************************\n");
}

void loop() {
  updateServoTest();
  printStatus();
  delay(10); // Small delay to prevent excessive CPU usage
}
