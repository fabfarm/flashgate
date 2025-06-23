#include <Arduino.h>
#include "WifiUtil.h"
#include "Util.h"

// constants
constexpr int gateOpenTime = 2000; // 2 seconds to keep the gate open

constexpr int lightThreshold = 500; // Threshold for light sensor to detect the beam (max value is 4095 for 12-bit ADC)

constexpr int lightSensorPin = 10;
constexpr int gateRelayPin = 4;

#define NEOPIXEL_PIN 48 // Onboard RGB LED data pin

constexpr char pattern[] = ".-.."; // Pattern to detect in the light sensor reading
constexpr int dotDuration = 100; // Duration of a dot in milliseconds
constexpr int dashDuration = 300; // Duration of a dash in milliseconds
constexpr int spaceDuration = 100; // Duration of a space between dots and dashes in milliseconds

int lastValue = 0; // Variable to store the last value read from the light sensor
String inputPattern = ""; // Buffer to store the detected pattern

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
    //Serial.print("Light Sensor Value: ");
    //Serial.println(lightValue); // Print the light sensor value to the serial monitor
    return lightValue;
}

bool detectBeam()
{
    // Read light sensor value
    int rawValue = getLightValue();
    boolean result = abs(rawValue - lastValue) > lightThreshold;
 
    if (result) {
      //Serial.println("BEAM MEE UPP!!\n");
    }
    else{
      lastValue = rawValue; // Update lastValue only if no beam is detected
    }
    return result;
}

char detectMorseSignal() {
  int startTime = millis();
  while (detectBeam()) {
    wait(10);  // Wait until the beam is no longer detected
  }
  int duration = millis() - startTime;
  Serial.print(duration);
  return (duration >= dashDuration) ? '-' : '.';
}


void detectPattern() {
  
  inputPattern += detectMorseSignal(); // Detect the Morse code signal and append it to inputPattern
  if (inputPattern.length() > strlen(pattern)) {
    inputPattern.remove(0, 1);
  }
}

// Main setup
void setup() {
  Serial.begin(115200);
  setupPins();
  setupWifi();

  lastValue = getLightValue(); // Initialize lastValue with the first reading from the light sensor
  neopixelWrite(NEOPIXEL_PIN, 128,0,128); // Set to purple (R=128, G=0, B=128)
}
    
// Main loop
void loop() {
  wait(spaceDuration); // Add a short delay to prevent excessive CPU usage
  detectPattern(); // Detect the Morse code pattern
  Serial.print("Detected Pattern: ");
  Serial.println(inputPattern); // Print the detected pattern to the serial monitor

  // if (detectBeam()) {
  //   openGate(); // Open the gate if the beam is detected
  //   while(detectBeam()) {
  //     // Wait until the beam is no longer detected
  //     wait(100); // Short delay to avoid busy-waiting
  //   }
  //   closeGate(); // Close the gate after the beam is no longer detected
  // }
}
