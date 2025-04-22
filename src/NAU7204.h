#ifndef _H
#define SCALE_H

#include <SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h>

bool nauSetup();
void nauTare(uint16_t samples = 64);
void nauCalibrate(float knownWeight, uint16_t samples = 64);
float nauRead(uint8_t sampleCount = 4);
void recordScaleSettings();
void readScaleSettings();

#endif
