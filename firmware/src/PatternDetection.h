#pragma once
#include <Arduino.h>
#include "Config.h"

int getLightValue();
bool detectBeam();
int detectSignalDuration();
char getMorseSymbol(int duration);
void countMorse(char symbol);
boolean checkMorse();
boolean checkCount();
void checkCountTimeout();
boolean detectPattern();
void initSensorLogic();

