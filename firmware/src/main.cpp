#include <Arduino.h>
#include <ESP32Servo.h>
#include "WifiManager.h"
#include "Webserver.h"
#include "PatternDetection.h"
#include "Config.h"

Servo patternServo;

void setupPins(){
  pinMode(lightSensorPin, INPUT);
  pinMode(gateRelayPin, OUTPUT);
  digitalWrite(gateRelayPin, LOW);
  pinMode(NEOPIXEL_PIN, OUTPUT);
  
  // Setup servo
  patternServo.attach(servoPin);
  patternServo.write(servoRestAngle); // Set servo to rest position
}

void closeGate() {
  digitalWrite(gateRelayPin, LOW);
  patternServo.write(servoRestAngle); // Move servo to 0 degrees
  neopixelWrite(NEOPIXEL_PIN, 255,0,0); // Set to red
  wait(2000); // Keep the gate closed for 2 seconds
  neopixelWrite(NEOPIXEL_PIN, 0,0,0); // Turn off the LED
}

void openGate() {
  digitalWrite(gateRelayPin, HIGH);
  patternServo.write(servoActiveAngle); // Move servo to 90 degrees
  neopixelWrite(NEOPIXEL_PIN, 0,255,0); // Set to green
}

// Manual control functions for web interface
void manualOpenGate() {
  Serial.println("Manual gate open triggered");
  openGate();
}

void manualCloseGate() {
  Serial.println("Manual gate close triggered");
  closeGate();
}

// Main setup
void setup() {
  Serial.begin(115200);
  Serial.println("Initialising...");
  setupPins();
  
  initSensorLogic(); // Initialize sensor logic variables
  setupWifi();
  setupWebServer();
  
  neopixelWrite(NEOPIXEL_PIN, 128,0,128); // Set to purple (R=128, G=0, B=128)
  Serial.println("GateRebooted iniated!");
}

// Main loop
void loop() {
  wait(100); // Add a short delay to prevent excessive CPU usage
  if (detectPattern()) {
    Serial.println("Pattern match!");
    Serial.println("BEAM MEE UPP!!\n");
    
    openGate();
    wait(gateOpenTime);
    closeGate();
  }
  
}
