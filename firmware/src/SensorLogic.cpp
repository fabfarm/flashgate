
#include "SensorLogic.h"
#include "Config.h"

int getLightValue()
{
    int lightValue = analogRead(lightSensorPin);
    Serial.print("L: ");
    Serial.println(lightValue);
    return lightValue;
}

bool detectBeam()
{
    int rawValue = getLightValue();
    boolean result = abs(rawValue - lastValue) > lightThreshold;
    if (!result) {
      lastValue = rawValue;
    }
    return result;
}

char detectMorseSignal() {
  int startTime = millis();
  while (detectBeam()) {
    wait(10);
    if (millis() - startTime > spaceDuration) {
      Serial.println("Spacetime continuum broken");
      return spaceChar;
    }
  }
  return (millis() - startTime >= dashDuration) ? dashChar : dotChar;
}

boolean detectPattern() {
  if (detectBeam()) {
    char signal = detectMorseSignal();
    initTime = millis();
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
    inputPattern += signal;
    if (inputPattern.length() > strlen(pattern)) {
      inputPattern.remove(0, 1);
    }
    Serial.print("Dot: ");
    Serial.print(dotCount);
    Serial.print(" Dash: ");
    Serial.print(dashCount);
    Serial.print(" M-Pattern: ");
    Serial.println(inputPattern);
    if (inputPattern.equals(pattern)) {
      inputPattern = "";
      Serial.println("Pattern matched!");
      Serial.println("BEAM MEE UPP!!\n");
      return true;
    }
  }
  return false;
}

boolean checkCount(){
  if (dotCount == maxDotCount && dashCount == maxDashCount) {
      initTime = millis();
    dotCount = 0;
    dashCount = 0;
    return true;
  }
  else if (millis() - initTime < timeoutDuration) {
    return false; //gota fix/change
  }
  return false;
}
