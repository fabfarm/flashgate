#include "PatternDetection.h"
#include "Config.h"

// Make the global pattern variable from main.cpp available here
extern char currentPattern[50];

// --- State Variables for Pattern Detection ---
enum State {
  IDLE,
  BEAM_BROKEN,
  TIMING_SIGNAL
};

State currentState = IDLE;
unsigned long signalStartTime = 0; // Time when the beam was first broken
unsigned long lastSignalTime = 0;  // Time the last signal (dot/dash) ended
char receivedPattern[50] = "";     // Buffer to store the incoming pattern
int receivedPatternIndex = 0;

// --- Function Implementations ---

/**
 * @brief Initializes the sensor logic, setting up the pin mode.
 */
void initSensorLogic() {
  pinMode(lightSensorPin, INPUT);
  if (debugPattern) {
    Serial.println("Pattern Detection: Initialized. Light sensor pin set to INPUT.");
  }
}

/**
 * @brief Reads the raw analog value from the light sensor.
 * @return The raw sensor value (0-4095).
 */
int getLightValue() {
  return analogRead(lightSensorPin);
}

/**
 * @brief Detects if the light beam is currently broken.
 * @return True if the light level is below the threshold, false otherwise.
 */
bool detectBeam() {
  int lightValue = getLightValue();
  bool broken = lightValue < lightThreshold;
  if (debugSensor) {
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 1000) {
      Serial.printf("Sensor Value: %d (Threshold: %d) -> Beam Broken: %s\n", lightValue, lightThreshold, broken ? "YES" : "NO");
      lastPrint = millis();
    }
  }
  return broken;
}

/**
 * @brief Converts a signal duration into a Morse symbol (dot or dash).
 * @param duration The duration of the signal in milliseconds.
 * @return The corresponding character ('.' for dot, '-' for dash, or 0 for invalid).
 */
char getMorseSymbol(int duration) {
  // A dot is anything shorter than the dot duration + a small tolerance
  if (duration > 50 && duration < (dotDuration + (dotDuration / 2))) {
    return dotChar;
  }
  // A dash is anything between the dot and dash durations
  else if (duration > (dashDuration - (dashDuration / 2)) && duration < (dashDuration + (dashDuration / 2))) {
    return dashChar;
  }
  return 0; // Invalid symbol
}

/**
 * @brief Checks if the received pattern matches the target pattern.
 * @return True if the patterns match, false otherwise.
 */
boolean checkMorse() {
  if (strcmp(receivedPattern, currentPattern) == 0) {
    if (debugPattern) {
      Serial.printf("Pattern Match! Received: '%s', Target: '%s'\n", receivedPattern, currentPattern);
    }
    return true;
  }
  return false;
}


/**
 * @brief The main pattern detection state machine. This should be called in the main loop.
 * @return True if the correct pattern has been detected, false otherwise.
 */
boolean detectPattern() {
  bool beamBroken = detectBeam();
  unsigned long currentTime = millis();

  // State machine logic
  switch (currentState) {
    case IDLE:
      if (beamBroken) {
        // Beam has just been broken, start timing
        signalStartTime = currentTime;
        currentState = BEAM_BROKEN;
        if (debugPattern) {
          Serial.println("State -> BEAM_BROKEN");
        }
      } else {
        // Check for pattern timeout while idle
        if (receivedPatternIndex > 0 && (currentTime - lastSignalTime > patternTimeout)) {
          if (debugPattern) {
            Serial.printf("Pattern timeout. Resetting pattern buffer. (Received: %s)\n", receivedPattern);
          }
          // Reset the pattern buffer
          memset(receivedPattern, 0, sizeof(receivedPattern));
          receivedPatternIndex = 0;
        }
      }
      break;

    case BEAM_BROKEN:
      if (!beamBroken) {
        // Beam is restored, signal has ended.
        unsigned long signalDuration = currentTime - signalStartTime;
        if (debugPattern) {
          Serial.printf("Signal ended. Duration: %lu ms\n", signalDuration);
        }

        char symbol = getMorseSymbol(signalDuration);
        if (symbol != 0) {
          // Valid symbol detected, add it to our received pattern
          if (receivedPatternIndex < sizeof(receivedPattern) - 1) {
            receivedPattern[receivedPatternIndex++] = symbol;
            receivedPattern[receivedPatternIndex] = '\0'; // Null-terminate
            lastSignalTime = currentTime;
            if (debugPattern) {
              Serial.printf("Symbol: '%c'. Current pattern: '%s'\n", symbol, receivedPattern);
            }
          }
        } else {
           if (debugPattern) {
              Serial.printf("Invalid signal duration (%lu ms). Ignoring.\n", signalDuration);
           }
        }

        currentState = IDLE;
        if (debugPattern) {
          Serial.println("State -> IDLE");
        }
      }
      break;

    // Default case to handle any unexpected state
    default:
      currentState = IDLE;
      break;
  }

  // After state logic, check if the pattern matches
  if (checkMorse()) {
    // Reset for next pattern
    memset(receivedPattern, 0, sizeof(receivedPattern));
    receivedPatternIndex = 0;
    currentState = IDLE;
    return true;
  }

  return false;
}

// NOTE: The following functions from the .h file are not used in this implementation
// as the logic is consolidated within the `detectPattern` state machine.
// They are here to satisfy the compiler if they were to be linked elsewhere.
int detectSignalDuration() { return 0; }
void countMorse(char symbol) {}
boolean checkCount() { return false; }
void checkCountTimeout() {}
