
#pragma once

#include <Arduino.h>

// Non-blocking delay utility
void wait(int miliseconds);

// Constants
constexpr int gateOpenTime = 2000; // 2 seconds to keep the gate open
constexpr int lightThreshold = 400; // Threshold for light sensor to detect the beam (max value is 4095 for 12-bit ADC)
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
constexpr int timeoutDuration = 2000; // 10 seconds timeout for detecting a valid pattern
constexpr int maxDotCount = 5; 
constexpr int maxDashCount = 2;

// Variables
extern int initTime; // Variable to store the initial time
extern int dotCount; // count for number of detected signals
extern int dashCount; // count for number of detected dashes
extern int lastValue; // Variable to store the last value read from the light sensor
extern String inputPattern; // Buffer to store the detected pattern


