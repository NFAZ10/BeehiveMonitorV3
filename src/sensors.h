#ifndef SENSORS_H
#define SENSORS_H



void tareScale();
void initScale();
void initDHTSensors();
void updateScale();
float movingAverage(int newValue);
void readDHTSensors();
void measureBattery();




#endif