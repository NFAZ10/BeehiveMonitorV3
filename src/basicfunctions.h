#ifndef BASICFUNCTIONS_H
#define BASICFUNCTIONS_H

#include <Adafruit_NeoPixel.h>

void initSerial();
void loadPreferences();
void clearPreferences();
void enterDeepSleep();
void enterNap();

extern Adafruit_NeoPixel strip;

#endif