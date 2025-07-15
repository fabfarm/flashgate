#include <Arduino.h>
#include "WifiManager.h"
#include "Webserver.h"
#include "SensorLogic.h"
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
  setupPins();
  
  initTime = millis(); // Initialize initTime with the current time
  lastValue = getLightValue(); // Initialize lastValue with the first reading from the light sensor

  setupWifi();
  setupWebServer();
  
  neopixelWrite(NEOPIXEL_PIN, 128,0,128); // Set to purple (R=128, G=0, B=128)
}
    
// Main loop
void loop() {
  wait(100); // Add a short delay to prevent excessive CPU usage
  if (detectPattern() || checkCount()) {
    Serial.println("Count reached!");
    Serial.println("BEAM MEE UPP!!\n");
    openGate();
    wait(gateOpenTime);
    closeGate();
  }
}
