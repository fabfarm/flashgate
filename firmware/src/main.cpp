#include <Arduino.h>
#include <ESP32Servo.h>
#include "WifiManager.h"
#include "Webserver.h"
#include "PatternDetection.h"
#include "Config.h"

Servo patternServo;

// Touch button and test mode variables
bool testMode = false;
unsigned long testModeStartTime = 0;
unsigned long lastTouchTime = 0;
int lastLightValue = 0;

void setupPins(){
  pinMode(lightSensorPin, INPUT);
  pinMode(gateRelayPin, OUTPUT);
  digitalWrite(gateRelayPin, LOW);
  pinMode(NEOPIXEL_PIN, OUTPUT);
  
  // Setup servo
  patternServo.attach(servoPin);
  patternServo.write(servoRestAngle); // Set servo to rest position
  
  // Setup touch button
  touchAttachInterrupt(touchButtonPin, [](){
    unsigned long currentTime = millis();
    if (currentTime - lastTouchTime > touchDebounceTime) {
      lastTouchTime = currentTime;
      if (!testMode) {
        testMode = true;
        testModeStartTime = currentTime;
        Serial.println("*** ENTERING TEST MODE ***");
        Serial.println("Touch button pressed! Any light changes will move servo for 30 seconds.");
        neopixelWrite(NEOPIXEL_PIN, 0, 0, 255); // Blue LED indicates test mode
      } else {
        testMode = false;
        Serial.println("*** EXITING TEST MODE ***");
        Serial.println("Touch button pressed! Returning to normal pattern detection mode.");
        neopixelWrite(NEOPIXEL_PIN, 0, 0, 0); // Turn off LED
        patternServo.write(servoRestAngle); // Return servo to rest
      }
    }
  }, touchThreshold);
  
  // Initialize light sensor baseline
  lastLightValue = analogRead(lightSensorPin);
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

// Servo test function to verify proper configuration
void testServo() {
  Serial.println("Testing servo movement...");
  
  // Test sequence: rest -> active -> rest -> sweep test
  Serial.println("Moving servo to rest position (0°)");
  patternServo.write(servoRestAngle);
  wait(1000);
  
  Serial.println("Moving servo to active position (90°)");
  patternServo.write(servoActiveAngle);
  wait(1000);
  
  Serial.println("Moving servo back to rest position (0°)");
  patternServo.write(servoRestAngle);
  wait(1000);
  
  // Sweep test to verify full range
  Serial.println("Performing sweep test...");
  for (int angle = 0; angle <= 180; angle += 30) {
    Serial.printf("Moving to %d degrees\n", angle);
    patternServo.write(angle);
    wait(500);
  }
  
  // Return to rest position
  Serial.println("Returning to rest position");
  patternServo.write(servoRestAngle);
  wait(1000);
  
  Serial.println("Servo test completed!");
}

// Test mode function - moves servo on any light change
void handleTestMode() {
  int currentLightValue = analogRead(lightSensorPin);
  int lightDifference = abs(currentLightValue - lastLightValue);
  
  // If light change is significant, move servo
  if (lightDifference > 50) { // Adjust sensitivity as needed
    Serial.printf("Light change detected! Value: %d -> %d (diff: %d)\n", 
                  lastLightValue, currentLightValue, lightDifference);
    
    // Alternate between active and rest positions
    static bool servoState = false;
    if (servoState) {
      patternServo.write(servoActiveAngle);
      Serial.println("Servo moved to ACTIVE position (90°)");
    } else {
      patternServo.write(servoRestAngle);
      Serial.println("Servo moved to REST position (0°)");
    }
    servoState = !servoState;
    
    // Flash the LED
    neopixelWrite(NEOPIXEL_PIN, 255, 255, 0); // Yellow flash
    wait(100);
    neopixelWrite(NEOPIXEL_PIN, 0, 0, 255); // Back to blue
  }
  
  lastLightValue = currentLightValue;
  
  // Check if test mode should timeout
  if (millis() - testModeStartTime > testModeTimeout) {
    testMode = false;
    Serial.println("*** TEST MODE TIMEOUT ***");
    Serial.println("Returning to normal pattern detection mode.");
    neopixelWrite(NEOPIXEL_PIN, 0, 0, 0); // Turn off LED
    patternServo.write(servoRestAngle); // Return servo to rest
  }
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
  
  // Test servo functionality before proceeding
  testServo();
  
  initSensorLogic(); // Initialize sensor logic variables
  setupWifi();
  setupWebServer();
  
  neopixelWrite(NEOPIXEL_PIN, 128,0,128); // Set to purple (R=128, G=0, B=128)
  Serial.println("GateRebooted iniated!");
  Serial.println("Touch button (GPIO14) ready - tap to enter/exit test mode!");
  Serial.println("In test mode: any light changes will move the servo for 30 seconds.");
}

// Main loop
void loop() {
  wait(100); // Add a short delay to prevent excessive CPU usage
  
  if (testMode) {
    // In test mode - any light change moves servo
    handleTestMode();
  } else {
    // Normal mode - pattern detection
    if (detectPattern()) {
      Serial.println("Pattern match!");
      Serial.println("BEAM MEE UPP!!\n");
      
      openGate();
      wait(gateOpenTime);
      closeGate();
    }
  }
}
