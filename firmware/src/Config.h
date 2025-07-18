#pragma once
#include <Arduino.h>

// Pins \\ ---
constexpr int lightSensorPin = 10;
constexpr int gateRelayPin = 4;
constexpr int NEOPIXEL_PIN = 48; // Onboard RGB LED data pin
// -----------

// Constants \\ ---

// Light Sensor settings
constexpr int lightThreshold = 500; // Threshold for light sensor to detect the beam (max value is 4095 for 12-bit ADC)

// Gate settings
constexpr int gateOpenTime = 2000; // duration to keep the gate open in milliseconds

// Pattern values
constexpr char pattern[] = ".-..--"; // Pattern to detect in the light sensor reading
constexpr int maxDotCount = 5; 
constexpr int maxDashCount = 2;

// Pattern detection settings
#define checkMorsePattern true // Enable or disable pattern detection (true/false)
#define checkCountPattern true // Enable or disable count detection (true/false)
constexpr int dotDuration = 200; // Duration of a dot in milliseconds               TODO not used??
constexpr int dashDuration = 600; // Duration of a dash in milliseconds  
constexpr int spaceDuration = 2000; // Duration of space between signals
constexpr int timeoutDuration = 5000; // timeout in case of false positives with light to reset current base reading
constexpr int patternTimeout = 3000; // timeout for pattern detection in milliseconds
constexpr char dotChar = '.';
constexpr char dashChar = '-';
constexpr char spaceChar = ' ';


// WIFI credentials
constexpr char ssid[] = "fabfarm";
constexpr char password[] = "imakestuff";
constexpr int maxAttempts = 30; //timeout attempts

// Debug settings
#define debugSensor false // Enable or disable debug mode for sensor readings (true/false)
#define debugPattern true // Enable or disable debug mode for pattern detection (true/false)

// -------------

// Non-blocking delay utility function
void wait(int miliseconds);