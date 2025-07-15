#include "Config.h"

int initTime = 0;
int dotCount = 0;
int dashCount = 0;
int lastValue = 0;
String inputPattern = "";

void wait(int miliseconds){
  uint32_t start = millis();
  while (millis() - start < miliseconds) {
    // Just wait (compares elapsed time with passed milliseconds)
    // hello darkness my old friend ...
  }
}
