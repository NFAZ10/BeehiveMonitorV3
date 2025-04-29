#include "variables.h"

// Global variable definitions
bool debug = true;
bool tareRequested = false;
bool nauCalRequested = false;
bool lowPowerMode = false;
bool useArduinoCloud = false;
bool forceUpdate = false;
bool nauAvailable = false;
bool disablesleep = false;
bool newSetup = true;
int reversedloadcell = 0;
String Name;


float h1 = -1, t1 = -1;
float h2 = -1, t2 = -1;
int last_weightstore = 0;
float weightInPounds = 0.0;
int last_weight = 0;
float calibrationValue = CALIBRATION_FACTOR;
float voltageDividerReading = 0.0;
float voltageCalibrationFactor = 1.31;
float voltageOffset = 0.0;


float vout = 0.0;
float vin = 0.0;
int value = 0;
unsigned long t = 0;

// OTA Variables
String otaBranch = "main"; // Default branch

const char* DEVversionURL = "https://raw.githubusercontent.com/NFAZ10/HiveMonitor/refs/heads/dev/src/version.txt";
const char* DEVfirmwareURL = "https://raw.githubusercontent.com/NFAZ10/HiveMonitor/dev/.pio/build/esp32dev/firmware.bin";
const char* DEVcurrentVersion = "1.2.3DEV";

const char* MAINversionURL = "https://raw.githubusercontent.com/NFAZ10/BeehiveMonitorV3/refs/heads/main/version.txt";
const char* MAINfirmwareURL = "https://raw.githubusercontent.com/NFAZ10/BeehiveMonitorV3/main/.pio/build/esp32dev/firmware.bin";
const char* MAINcurrentVersion = "3.3.4";

const char* versionURL;
const char* firmwareURL;
const char* currentVersion;

// MQTT Server Detail
const char* mqttServer = "mqtt.beehivemonitor.com";
const char* mqttUser = "Hivelogin";
const char* mqttPassword = "824656789";

// Other Variables
unsigned long lastPublishTime = 0; // Initialize outside the loop
const unsigned long publishInterval = 30000; // 30 seconds
int awsConnectAttempts = 0;
