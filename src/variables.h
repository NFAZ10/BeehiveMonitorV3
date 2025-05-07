#ifndef VARIABLES_H
#define VARIABLES_H

#include <Arduino.h>

// Debug levels
#define DEBUG_LEVEL_NONE 0
#define DEBUG_LEVEL_ERROR 1
#define DEBUG_LEVEL_INFO 2
#define DEBUG_LEVEL_VERBOSE 3

// Set current debug level
#define CURRENT_DEBUG_LEVEL DEBUG_LEVEL_INFO

// Debug macros
#if CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_ERROR
  #define DEBUG_ERROR(x) Serial.println("[ERROR] " x)
#else
  #define DEBUG_ERROR(x)
#endif

#if CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_INFO
  #define DEBUG_INFO(x) Serial.println("[INFO] " x)
#else
  #define DEBUG_INFO(x)
#endif

#if CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE
  #define DEBUG_VERBOSE(x) Serial.println("[VERBOSE] " x)
#else
  #define DEBUG_VERBOSE(x)
#endif

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

// Memory optimization
#define PREFERENCES_NAMESPACE "beehive"
#define PREFERENCES_READ_ONLY false

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
extern bool nauAvailable;
extern int heartbeat;

extern String Name;
extern String topicBase;

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
