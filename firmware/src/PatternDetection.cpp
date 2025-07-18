#include "PatternDetection.h"
#include "WebServer.h"
#include "Config.h"

// Variables
int lastCountUpdate = 0;
int dotCount = 0;
int dashCount = 0;
int lastValue = 0; // Initialize lastValue with the first reading from the light sensor
char nullChar = '\0';
String inputPattern = "";
int initTime = millis();

// returns the absolute value of the sensor pin reading
int getLightValue()
{
    int lightValue = analogRead(lightSensorPin);
#if debugSensor // debugging print
    Serial.print("L: ");
    Serial.println(lightValue);
#endif
    return lightValue;
}

void sendSensorReadings()
{
    if (millis() - initTime < 300)
    {
        return;
    }
    int lightValue = getLightValue();
    sendData("light", String(lightValue));
    initTime = millis(); // Reset initTime to avoid sending too frequently
}

// returns true if light threshold is exceeded, only updates saved value if not exceeded
bool detectBeam()
{
    int rawValue = getLightValue();
    boolean result = abs(rawValue - lastValue) > lightThreshold;
    if (!result)
    {
        lastValue = rawValue;
        neopixelWrite(NEOPIXEL_PIN, 0, 0, 0); // turn off
    }
    else
    {
        neopixelWrite(NEOPIXEL_PIN, 255, 255, 0); // Set to yellow
    }
    sendSensorReadings(); // Send sensor readings periodically
    return result;
}

// Returns the duration of the detected beam signal in milliseconds, includes timeout
int detectSignalDuration()
{
    int startTime = millis();
    while (detectBeam())
    {
        wait(10);
        if (millis() - startTime > timeoutDuration)
        {
            lastValue = getLightValue(); // Update lastValue to avoid false positives in case of timeout
            return timeoutDuration; // Spacetime continuum broken
        }
    }
    return millis() - startTime;
}

char getMorseSymbol(int duration)
{
    if (duration <= 0)
    {
        return nullChar;
    }
    return (duration >= spaceDuration) ? (spaceChar) : ((duration >= dashDuration) ? (dashChar) : (dotChar));
}

void countMorse(char symbol)
{
    switch (symbol)
    {
    case dotChar:
        dotCount++;
        break;
    case dashChar:
        dashCount++;
        break;
    default:
        break;
    }
    lastCountUpdate = millis(); // Reset lastUpdate to current time
}

// returns true if matching morse pattern is detected
boolean checkMorse()
{
    // Check if pattern matches
    if (inputPattern.equals(pattern))
    {
        inputPattern = "";
        return true;
    }
    return false;
}

void resetCount()
{
    dotCount = 0;
    dashCount = 0;
    sendData("dots", String(dotCount));
    sendData("dashes", String(dashCount));
}

// returns true if count of dots and dashes matches the expected values
boolean checkCount()
{
    if (dotCount == maxDotCount && dashCount == maxDashCount)
    {
        resetCount(); // Reset count after successful match
        return true;
    }
    return false;
}

void checkCountTimeout()
{
    if (millis() - lastCountUpdate >= patternTimeout)
    {
        resetCount(); // Reset count if timeout occurs
    }
}

boolean detectPattern()
{
    checkCountTimeout(); // Check if count timeout has occurred
    if (detectBeam())
    {
        int duration = detectSignalDuration();
        char symbol = getMorseSymbol(duration); // Get morse symbol
        if (symbol == nullChar)
        {
            return false;
        } // No valid signal detected
        if (inputPattern.length() > strlen(pattern))
        {
            inputPattern.remove(0, 1);
        } // Remove the oldest char from the string
        inputPattern += symbol; // Add new char to pattern
        countMorse(symbol);     // Update the count

        #if debugPattern // debugging print
        Serial.print("Dot: ");
        Serial.print(dotCount);
        Serial.print(" Dash: ");
        Serial.print(dashCount);
        Serial.print(" Morse-Pattern: ");
        Serial.print(inputPattern);
        Serial.print(" Time: ");
        Serial.println(duration);
        #endif

        sendData("dots", String(dotCount));
        sendData("dashes", String(dashCount));
        sendData("pattern", inputPattern);
        sendData("time", String(duration));
        // sendData("light", String(getLightValue()));

        #if checkMorsePattern
        if (checkMorse())
        {
            return true;
        }
        #endif

        #if checkCountPattern
        if (checkCount())
        {
            return true;
        }
        #endif
    }
    return false; // No pattern matched
}

void initSensorLogic()
{
    dotCount = 0;
    dashCount = 0;
    inputPattern = "";
    lastValue = getLightValue(); // Initialize lastValue with the first reading from the light sensor
}
