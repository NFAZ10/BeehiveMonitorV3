#ifndef BASICFUNCTIONS_H
#define BASICFUNCTIONS_H



void initSerial();
void loadPreferences();
void clearPreferences();
void putPreferences();
void printPreferences();
void enterDeepSleep(uint64_t sleepTimeSeconds);
void enterLightSleep(uint64_t sleepTimeSeconds);



#endif