#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include "WifiManager.h"
#include "Webserver.h"
#include "PatternDetection.h"
#include "Config.h"

// Forward declarations
void openGate();
void closeGate();
void testServoMovement();
void printHardwareStatus();
void printGPIOStatus();

Servo patternServo;

// Touch button and gate control variables
unsigned long lastTouchTime = 0;
bool gateOpenedByTouch = false;
unsigned long gateOpenTime_touch = 0;
volatile bool touchButtonPressed = false; // Flag for ISR

void setupPins(){
  Serial.println("=== SETTING UP PINS ===");
  
  pinMode(lightSensorPin, INPUT);
  Serial.printf("Light sensor configured on pin %d\n", lightSensorPin);
  
  pinMode(gateRelayPin, OUTPUT);
  digitalWrite(gateRelayPin, LOW);
  Serial.printf("Gate relay configured on pin %d (initially LOW)\n", gateRelayPin);
  
  pinMode(NEOPIXEL_PIN, OUTPUT);
  Serial.printf("NeoPixel configured on pin %d\n", NEOPIXEL_PIN);
  
  // Setup servo
  Serial.printf("Attaching servo to pin %d...\n", servoPin);
  patternServo.attach(servoPin);
  patternServo.write(servoRestAngle);
  Serial.printf("Servo set to rest position: %d degrees\n", servoRestAngle);
  delay(100); // Give servo time to move
  
  // Setup touch button
  Serial.printf("Setting up touch button on pin %d (threshold: %d)\n", touchButtonPin, touchThreshold);
  touchAttachInterrupt(touchButtonPin, [](){
    // Minimal ISR - just set a flag
    touchButtonPressed = true;
  }, touchThreshold);
  
  Serial.println("=== PIN SETUP COMPLETE ===\n");
}

// Gate state management
bool gateCloseInProgress = false;
unsigned long gateCloseStartTime = 0;
const unsigned long gateCloseHoldTime = 2000; // 2 seconds

void closeGate() {
  Serial.println("--- CLOSING GATE ---");
  
  digitalWrite(gateRelayPin, LOW);
  Serial.printf("Relay set to LOW (pin %d)\n", gateRelayPin);
  
  patternServo.write(servoRestAngle);
  Serial.printf("Servo moving to rest position: %d degrees\n", servoRestAngle);
  
  neopixelWrite(NEOPIXEL_PIN, 255,0,0); // Set to red
  Serial.println("NeoPixel set to RED (gate closed)");
  
  // Start non-blocking close sequence
  gateCloseInProgress = true;
  gateCloseStartTime = millis();
}

void updateGateClose() {
  if (!gateCloseInProgress) return;
  
  unsigned long currentTime = millis();
  if (currentTime - gateCloseStartTime >= gateCloseHoldTime) {
    Serial.println("Gate held closed for 2 seconds");
    neopixelWrite(NEOPIXEL_PIN, 0,0,0); // Turn off the LED
    Serial.println("NeoPixel turned OFF");
    Serial.println("--- GATE CLOSED ---\n");
    gateCloseInProgress = false;
  }
}

void openGate() {
  Serial.println("--- OPENING GATE ---");
  
  digitalWrite(gateRelayPin, HIGH);
  Serial.printf("Relay set to HIGH (pin %d)\n", gateRelayPin);
  
  patternServo.write(servoActiveAngle);
  Serial.printf("Servo moving to active position: %d degrees\n", servoActiveAngle);
  
  neopixelWrite(NEOPIXEL_PIN, 0,255,0); // Set to green
  Serial.println("NeoPixel set to GREEN (gate open)");
  Serial.println("--- GATE OPENED ---\n");
}

// Manual control functions for web interface
void manualOpenGate() {
  Serial.println(">>> MANUAL GATE OPEN triggered via web interface");
  openGate();
}

void manualCloseGate() {
  Serial.println(">>> MANUAL GATE CLOSE triggered via web interface");
  closeGate();
}

// Servo test function - sweeps through full range
// Non-blocking servo test variables
bool servoTestActive = false;
int servoTestIndex = 0;
unsigned long servoTestLastMove = 0;
const unsigned long servoMoveDelay = 1000; // 1 second between moves
int testPositions[] = {0, 30, 60, 90, 120, 150, 180, 90, 0};
const int numTestPositions = sizeof(testPositions) / sizeof(testPositions[0]);

void testServoMovement() {
  if (!servoTestActive) {
    Serial.println("\n=== SERVO MOVEMENT TEST ===");
    Serial.println("Testing servo range and movement...");
    servoTestActive = true;
    servoTestIndex = 0;
    servoTestLastMove = millis();
    
    // Start with first position
    Serial.printf("Moving servo to %d degrees...\n", testPositions[0]);
    patternServo.write(testPositions[0]);
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
      patternServo.write(testPositions[servoTestIndex]);
      servoTestLastMove = currentTime;
    } else {
      // Test complete
      patternServo.write(servoRestAngle);
      Serial.printf("Servo returned to rest position: %d degrees\n", servoRestAngle);
      Serial.println("=== SERVO TEST COMPLETE ===\n");
      servoTestActive = false;
    }
  }
}

// Hardware status monitoring
void printHardwareStatus() {
  Serial.println("\n=== DETAILED HARDWARE STATUS ===");
  
  // Memory status
  Serial.printf("Free Heap: %d bytes (%.1f%%)\n", ESP.getFreeHeap(), 
    (float)ESP.getFreeHeap() / ESP.getHeapSize() * 100);
  Serial.printf("Largest Free Block: %d bytes\n", ESP.getMaxAllocHeap());
  Serial.printf("Total Heap: %d bytes\n", ESP.getHeapSize());
  Serial.printf("Min Free Heap: %d bytes\n", ESP.getMinFreeHeap());
  
  // Power and temperature
  Serial.printf("CPU Temperature: %.1f°C\n", temperatureRead());
  
  // Flash information
  Serial.printf("Flash Size: %d bytes\n", ESP.getFlashChipSize());
  Serial.printf("Sketch Size: %d bytes\n", ESP.getSketchSize());
  Serial.printf("Free Sketch Space: %d bytes\n", ESP.getFreeSketchSpace());
  
  // WiFi status
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("WiFi SSID: %s\n", WiFi.SSID().c_str());
    Serial.printf("WiFi RSSI: %d dBm\n", WiFi.RSSI());
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("DNS: %s\n", WiFi.dnsIP().toString().c_str());
    Serial.printf("Channel: %d\n", WiFi.channel());
  } else {
    Serial.println("WiFi: Disconnected");
  }
  
  printGPIOStatus();
  Serial.println("=== END HARDWARE STATUS ===\n");
}

// GPIO status monitoring
void printGPIOStatus() {
  Serial.println("\n--- GPIO STATUS ---");
  
  // Light sensor
  int lightValue = analogRead(lightSensorPin);
  Serial.printf("Light Sensor (GPIO%d): %d (%.2fV)\n", 
    lightSensorPin, lightValue, lightValue * 3.3 / 4095.0);
  
  // Touch sensor
  int touchValue = touchRead(touchButtonPin);
  Serial.printf("Touch Sensor (GPIO%d): %d (threshold: %d)\n", 
    touchButtonPin, touchValue, touchThreshold);
  
  // Gate relay
  int relayState = digitalRead(gateRelayPin);
  Serial.printf("Gate Relay (GPIO%d): %s\n", 
    gateRelayPin, relayState ? "HIGH (Active)" : "LOW (Inactive)");
  
  // Other GPIO readings
  Serial.printf("GPIO2 (ADC): %d\n", analogRead(2));
  Serial.printf("GPIO3 (ADC): %d\n", analogRead(3));
  Serial.printf("GPIO4 (ADC): %d\n", analogRead(4));
  
  Serial.println("--- END GPIO STATUS ---");
}

// Main setup
void setup() {
  Serial.begin(115200);
  delay(1000); // Give serial time to initialize and stabilize
  
  // Force output even if no serial monitor is connected
  Serial.println("\n");
  Serial.flush();
  
  Serial.println("==================================================");
  Serial.println("        ESP32 FLASHGATE FIRMWARE v2.0");
  Serial.println("==================================================");
  
  // Hardware information
  Serial.printf("Chip Model: %s\n", ESP.getChipModel());
  Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
  Serial.printf("CPU Cores: %d\n", ESP.getChipCores());
  Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Flash Size: %d bytes (%.2f MB)\n", ESP.getFlashChipSize(), ESP.getFlashChipSize() / 1024.0 / 1024.0);
  Serial.printf("Flash Speed: %d Hz\n", ESP.getFlashChipSpeed());
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Total Heap: %d bytes\n", ESP.getHeapSize());
  Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
  Serial.printf("Arduino Core Version: %s\n", ESP.getSdkVersion());
  
  // MAC addresses
  Serial.printf("WiFi MAC: %s\n", WiFi.macAddress().c_str());
  
  // Reset reason
  esp_reset_reason_t reset_reason = esp_reset_reason();
  Serial.printf("Reset Reason: ");
  switch(reset_reason) {
    case ESP_RST_POWERON: Serial.println("Power-on reset"); break;
    case ESP_RST_EXT: Serial.println("External reset"); break;
    case ESP_RST_SW: Serial.println("Software reset"); break;
    case ESP_RST_PANIC: Serial.println("Exception/panic reset"); break;
    case ESP_RST_INT_WDT: Serial.println("Interrupt watchdog reset"); break;
    case ESP_RST_TASK_WDT: Serial.println("Task watchdog reset"); break;
    case ESP_RST_WDT: Serial.println("Other watchdog reset"); break;
    case ESP_RST_DEEPSLEEP: Serial.println("Deep sleep reset"); break;
    case ESP_RST_BROWNOUT: Serial.println("Brownout reset"); break;
    case ESP_RST_SDIO: Serial.println("SDIO reset"); break;
    default: Serial.printf("Unknown reset (%d)\n", reset_reason); break;
  }
  
  Serial.println("==================================================");
  Serial.flush();
  
  Serial.println("Initializing hardware...");
  setupPins();
  
  initSensorLogic(); // Initialize sensor logic variables
  Serial.println("Sensor logic initialized");
  
  Serial.println("Starting WiFi connection...");
  setupWifi();
  
  Serial.println("Setting up web server...");
  setupWebServer();
  
  neopixelWrite(NEOPIXEL_PIN, 128,0,128); // Set to purple (R=128, G=0, B=128)
  Serial.println("Status LED set to PURPLE (system ready)");
  
  // Test servo movement on startup - TEMPORARILY DISABLED to debug boot issues
  // Serial.println("Performing startup servo test...");
  // testServoMovement();
  
  Serial.println("\n**************************************************");
  Serial.println("       FLASHGATE SYSTEM READY!");
  Serial.println("**************************************************");
  Serial.println("Touch button (GPIO14) ready - tap to test gate!");
  Serial.println("Pattern detection active");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Web interface available at: http://%s\n", WiFi.localIP().toString().c_str());
  }
  Serial.println("**************************************************\n");
  Serial.flush();
}

// Main loop
void loop() {
  static unsigned long lastStatusPrint = 0;
  static unsigned long lastDetailedStatus = 0;
  static unsigned long statusInterval = 10000; // Print status every 10 seconds (faster for debugging)
  static unsigned long detailedInterval = 60000; // Detailed status every 60 seconds
  
  unsigned long currentTime = millis();
  
  // Handle touch button press (moved out of ISR)
  if (touchButtonPressed) {
    touchButtonPressed = false; // Clear flag
    
    if (currentTime - lastTouchTime > touchDebounceTime) {
      lastTouchTime = currentTime;
      Serial.println("\n*** TOUCH BUTTON PRESSED ***");
      Serial.printf("Touch detected at %lu ms\n", currentTime);
      Serial.printf("Touch value: %d\n", touchRead(touchButtonPin));
      Serial.println("Opening gate to test servo and relay...");
      
      // Trigger gate opening sequence (tests both relay and servo)
      openGate();
      gateOpenedByTouch = true;
      gateOpenTime_touch = currentTime;
    } else {
      Serial.printf("Touch ignored (debounce) - only %lu ms since last touch\n", currentTime - lastTouchTime);
    }
  }
  
  // Update non-blocking servo test
  updateServoTest();
  
  // Update non-blocking gate close
  updateGateClose();
  
  // Print quick status updates
  if (currentTime - lastStatusPrint > statusInterval) {
    Serial.println("\n--- QUICK STATUS ---");
    Serial.printf("Uptime: %lu seconds\n", currentTime / 1000);
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Temperature: %.1f°C\n", temperatureRead());
    Serial.printf("Light Sensor: %d\n", analogRead(lightSensorPin));
    Serial.printf("Touch Value: %d\n", touchRead(touchButtonPin));
    Serial.printf("WiFi: %s", WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf(" (RSSI: %d dBm)", WiFi.RSSI());
    }
    Serial.println();
    Serial.println("--- END QUICK STATUS ---\n");
    lastStatusPrint = currentTime;
  }
  
  // Print detailed hardware status
  if (currentTime - lastDetailedStatus > detailedInterval) {
    printHardwareStatus();
    lastDetailedStatus = currentTime;
  }
  
  delay(100); // Add a short delay to prevent excessive CPU usage
  
  // Check if gate was opened by touch button and needs to be closed
  if (gateOpenedByTouch && (currentTime - gateOpenTime_touch > gateOpenTime)) {
    Serial.printf("Touch gate timeout reached (%lu ms), closing gate...\n", gateOpenTime);
    closeGate();
    gateOpenedByTouch = false;
  }
  
// Pattern-triggered gate state management
bool gateOpenedByPattern = false;
unsigned long gateOpenTime_pattern = 0;

// Normal pattern detection (only if gate is not currently open from touch)
  if (!gateOpenedByTouch && !gateOpenedByPattern && detectPattern()) {
    Serial.println("\n!!! PATTERN DETECTED !!!");
    Serial.println("BEAM MEE UPP!!");
    
    openGate();
    gateOpenedByPattern = true;
    gateOpenTime_pattern = millis();
  }
  
  // Check if gate was opened by pattern and needs to be closed
  if (gateOpenedByPattern && (millis() - gateOpenTime_pattern > gateOpenTime)) {
    Serial.printf("Pattern gate timeout reached (%lu ms), closing gate...\n", gateOpenTime);
    closeGate();
    gateOpenedByPattern = false;
    Serial.println("Pattern detection cycle complete\n");
  }
}
