#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <HX711_ADC.h>


void tareScale();
void initScale();
void initDHTSensors();
void updateScale();
float movingAverage(int newValue);
void readDHTSensors();
void measureBattery();
void recalibrateScale();
void handleSerialCommands();
void processCommand();
void reverseloadcell() ;

extern HX711_ADC LoadCell();


#endif

