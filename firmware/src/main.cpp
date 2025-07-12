#include <Arduino.h>
#include "WifiUtil.h"
#include "Util.h"
#include <LittleFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// constants
constexpr int gateOpenTime = 2000; // 2 seconds to keep the gate open

constexpr int lightThreshold = 500; // Threshold for light sensor to detect the beam (max value is 4095 for 12-bit ADC)

constexpr int lightSensorPin = 10;
constexpr int gateRelayPin = 4;

#define NEOPIXEL_PIN 48 // Onboard RGB LED data pin


constexpr char pattern[] = ".-..--"; // Pattern to detect in the light sensor reading

constexpr int dotDuration = 200; // Duration of a dot in milliseconds
constexpr int dashDuration = 600; // Duration of a dash in milliseconds
constexpr int spaceDuration = 3000; 

constexpr char dashChar = '-';
constexpr char dotChar = '.';
constexpr char spaceChar = ' ';

int initTime = 0; // Variable to store the initial time
constexpr int timeoutDuration = 2000; // 10 seconds timeout for detecting a valid pattern
int dotCount = 0; // count for number of detected signals
int dashCount = 0; // count for number of detected dashes
constexpr int maxDotCount = 5; 
constexpr int maxDashCount = 2;

int lastValue = 0; // Variable to store the last value read from the light sensor
String inputPattern = ""; // Buffer to store the detected pattern

AsyncWebServer server(80);

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

int getLightValue()
{
    // Read the light sensor value
    int lightValue = analogRead(lightSensorPin);
    return lightValue;
}

bool detectBeam()
{
    // Read light sensor value
    int rawValue = getLightValue();
    boolean result = abs(rawValue - lastValue) > lightThreshold;
    
    if (!result) {
      lastValue = rawValue; // Update lastValue only if no beam is detected
    }

    return result;
}

char detectMorseSignal() {
  int startTime = millis();
  while (detectBeam()) {
    wait(10);  // Wait until the beam is no longer detected
  }

  if (millis() - startTime > spaceDuration) {
      return spaceChar; // If the beam is detected for too long, return a space
  }
  return (millis() - startTime >= dashDuration) ? dashChar : dotChar;
}


void detectPattern() {
  
  if (detectBeam()) {

    char signal = detectMorseSignal(); // Detect the Morse code signal
    initTime = millis(); // Reset the initial time when the beam is detected

    switch (signal) {
      case dotChar:
        dotCount++;
        break;
      case dashChar:
        dashCount++;
        break;
      default:
        break;
    }
 
    inputPattern += signal; // Detect the Morse code signal and append it to inputPattern

    if (inputPattern.length() > strlen(pattern)) {
      inputPattern.remove(0, 1);
    }
    Serial.print("Dot: ");
    Serial.print(dotCount);
    Serial.print(" Dash: ");
    Serial.print(dashCount);
    Serial.print(" M-Pattern: ");
    Serial.println(inputPattern); // Print the detected pattern to the serial monitor

    // Compare inputPattern
    if (inputPattern.equals(pattern)) {
      Serial.println("Pattern matched!");
      Serial.println("BEAM MEE UPP!!\n");
      inputPattern = ""; // Reset after successful match

      openGate();
      wait(gateOpenTime);
      closeGate();
    }
  }
}

void checkCount(){
  if (dotCount == maxDotCount && dashCount == maxDashCount) {
    Serial.println("Count reached!");
    Serial.println("BEAM MEE UPP!!\n");
    openGate(); // Open the gate
    wait(gateOpenTime);
    closeGate();
  }
  else if (millis() - initTime < timeoutDuration) { //return if within timeout  
    return;
  }

  initTime = millis(); // Reset the initial time
  dotCount = 0; // reset counts
  dashCount = 0;
}

void setupWebServer() {
  if (!LittleFS.begin(true)) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  Serial.println("LittleFS mounted. Files:");
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  while (file) {
    Serial.println(file.name());
    file = root.openNextFile();
  }

  // Register dynamic endpoint first!
  server.on("/light", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = String("{\"value\":") + lastValue + ",\"lastValue\":" + lastValue + "}";
    request->send(200, "application/json", json);
  });

  // Then serve static files
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  server.begin();
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
  wait(10); // Add a short delay to prevent excessive CPU usage
  detectPattern(); // Detect the Morse code pattern
  checkCount(); // Check if the count has reached the desired values
}
