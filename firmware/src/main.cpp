#include <Arduino.h>
#include "WifiManager.h"
#include "Webserver.h"
#include "PatternDetection.h"
#include "Config.h"

void setupPins(){
  pinMode(lightSensorPin, INPUT);
  pinMode(gateRelayPin, OUTPUT);
  digitalWrite(gateRelayPin, LOW);
  pinMode(NEOPIXEL_PIN, OUTPUT);
}

void closeGate() {
  digitalWrite(gateRelayPin, LOW);
  neopixelWrite(NEOPIXEL_PIN, 255,0,0); // Set to red
}

void openGate() {
  digitalWrite(gateRelayPin, HIGH);
  neopixelWrite(NEOPIXEL_PIN, 0,255,0); // Set to green
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

int initTime = millis();

void sendSensorReadings() {
  if (millis() - initTime < 300) { return; }
  int lightValue = getLightValue();
  sendData("light", String(lightValue));
  initTime = millis(); // Reset initTime to avoid sending too frequently
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
  sendSensorReadings(); // Send sensor readings periodically
}
