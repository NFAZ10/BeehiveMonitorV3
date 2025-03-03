#ifndef BASICFUNCTIONS_H
#define BASICFUNCTIONS_H

#include <Adafruit_NeoPixel.h>

void initSerial();
void loadPreferences();
void clearPreferences();
void putPreferences();
  
void enterDeepSleep(uint64_t sleepTimeSeconds);
void enterLightSleep(uint64_t sleepTimeSeconds);

extern Adafruit_NeoPixel strip;

#endif