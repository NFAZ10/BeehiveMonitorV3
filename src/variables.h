#ifndef VARIABLES_H
#define VARIABLES_H

#include <Arduino.h>

// Constants and definitions
#define TARE_BUTTON_PIN        25
#define FactoryReset_PIN       33
#define button3                34
#define activate_monitor       35
#define VOLTAGE_PIN            32 // GPIO pin connected to the voltage divider
#define NUM_SAMPLES            500
#define MAX_DATA_POINTS        144
//#define CALIBRATION_FACTOR     29.17 // Use if using 4 CELLS
#define CALIBRATION_FACTOR     14.45 // Use if using 2 CELLS
#define SLEEP_INTERVAL_LOW_POWER 21600 // 6 hours
#define SLEEP_INTERVAL_NORMAL    3600  // 1 hour
#define LED_PIN    15
#define LED_COUNT 1



// Global variable declarations (extern)
extern bool debug;
extern bool tareRequested;
extern bool lowPowerMode;
extern bool useArduinoCloud;
extern bool forceUpdate;
extern bool weightset;
extern bool disablesleep;
extern bool newSetup;
extern int reversedloadcell;
extern bool nauCalRequested;

extern String Name;

extern float h1, t1;
extern float h2, t2;
extern int last_weightstore;
extern float weightInPounds;
extern int last_weight;
extern float calibrationValue;
extern float voltageDividerReading;
extern float voltageCalibrationFactor;
extern float voltageOffset;


extern float vout;
extern float vin;
extern int value;
extern unsigned long t;

// OTA Variables
extern String otaBranch;

extern const char* DEVversionURL;
extern const char* DEVfirmwareURL;
extern const char* DEVcurrentVersion;

extern const char* MAINversionURL;
extern const char* MAINfirmwareURL;
extern const char* MAINcurrentVersion;

extern const char* versionURL;
extern const char* firmwareURL;
extern const char* currentVersion;

// MQTT Server Details
extern const char* mqttServer;
extern const int mqttPort;
extern const char* mqttUser;
extern const char* mqttPassword;

// Other Variables
extern unsigned long lastPublishTime;
extern const unsigned long publishInterval;
extern int awsConnectAttempts;

#endif // VARIABLES_H
