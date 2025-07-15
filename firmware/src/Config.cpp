#include "Config.h"

// Non-blocking delay utility function
void wait(int miliseconds){
  uint32_t start = millis();
  while (millis() - start < miliseconds) {
    // Just wait (compares elapsed time with passed milliseconds)
    // hello darkness my old friend ...
  }
}
