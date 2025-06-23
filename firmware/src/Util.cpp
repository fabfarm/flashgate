#include <Arduino.h>

// This library provides utility functions for various tasks

//Uses non blocking delay code
void wait(int miliseconds){
  uint32_t start = millis();
  while (millis() - start < miliseconds) {
    // Just wait (compares elapsed time with passed milliseconds)
    // hello darkness my old friend ...
  }
}
