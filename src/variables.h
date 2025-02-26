#include <Arduino.h>


// Constants and definitions
#define TARE_BUTTON_PIN        25
#define VOLTAGE_PIN            32 // GPIO pin connected to the voltage divider
#define NUM_SAMPLES            500
#define MAX_DATA_POINTS        144
#define CALIBRATION_FACTOR     29.17 // Default calibration factor
#define SLEEP_INTERVAL_LOW_POWER 21600 // 6 hours
#define SLEEP_INTERVAL_NORMAL    3600  // 1 hour
#define LED_PIN    12

#define LED_COUNT 1



bool debug = true;
bool tareRequested = false;
bool lowPowerMode = false;
bool useArduinoCloud = false;



float h1 = -1, t1 = -1;
float h2 = -1, t2 = -1;
int last_weightstore = 0;
float weightInPounds = 0.0;
int last_weight = 0;
float calibrationValue = CALIBRATION_FACTOR;
String calibrationStatus = "";
String wifiSSID = "";
String wifiPassword = "";
float voltageDividerReading = 0.0;
float voltageCalibrationFactor = 1.31;
float voltageOffset = 0.0;

float vout = 0.0;
float vin = 0.0;
int value = 0;
unsigned long t = 0;

 String otaBranch = "main"; // Default branch

 const char* DEVversionURL = "https://raw.githubusercontent.com/NFAZ10/HiveMonitor/refs/heads/dev/src/version.txt";
 const char* DEVfirmwareURL = "https://raw.githubusercontent.com/NFAZ10/HiveMonitor/dev/.pio/build/esp32dev/firmware.bin";
 const char* DEVcurrentVersion = "1.2.3DEV";

 const char* MAINversionURL = "https://raw.githubusercontent.com/NFAZ10/HiveMonitor/refs/heads/webtermial/src/version.txt";
 const char* MAINfirmwareURL = "https://raw.githubusercontent.com/NFAZ10/HiveMonitor/webtermial/.pio/build/esp32dev/firmware.bin";
 const char* MAINcurrentVersion = "2.0.81";


 

const  char* versionURL;
const  char* firmwareURL;
const  char* currentVersion;



// Define the local MQTT server details
const char* mqttServer = "mqtt.beehivemonitor.com"; // Replace with your local MQTT server IP address
const int mqttPort = 4116; // Replace with your local MQTT server port
const char* mqttUser = "Hivelogin"; // Replace with your MQTT username
const char* mqttPassword = "824656789"; // Replace with your MQTT password





unsigned long lastPublishTime = 0; // Initialize outside the loop
const unsigned long publishInterval = 30000; // 15 minutes in milliseconds
int awsConnectAttempts = 0;


