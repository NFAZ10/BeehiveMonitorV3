#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "webserialsetup.h"
/////////////////
///Our Setups////
/////////////////

#include "basicfunctions.h"
#include "sensors.h"
#include "variables.h"
#include "ota.h"
#include "mqtt.h"
#include <Preferences.h>
#include "OLED.h"
#include "LoRaTransmitterSerial.h"
#include "NAU7204.h"

#define DEBUG 0
#if DEBUG
  #define DEBUG_PRINT(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
#endif

/////////////////

float battery = 0.0;
float grams = 0.0;
float temp1 = NAN;
float temp2 = NAN;
float weight = 0.0;
int counter = 0;
float mVA;
bool weightset = false;
int timesincelastrestart = 0;
float oldbattery = 0.0;
bool charging = false;
extern String Name;

const float TEMP_SENSITIVITY = 190;  // grams per degree Celsius
const float T_BASELINE = 0.1;          // temp at calibration time

Preferences pref;

void IRAM_ATTR tareButtonISR() {
    tareRequested = true; // Set the flag when the interrupt is triggered
}

void IRAM_ATTR activate_monitorISR() {
    newSetup = false; // Set the flag when the interrupt is triggered
}
void IRAM_ATTR factoryResetISR() {
    Serial.println("Factory Reset Button Pressed");
    newSetup = true; // Set the flag when the interrupt is triggered
    clearPreferences();
    
    //ESP.restart();
}

void setup() {
    // Your other setup code
    initSerial();
    setupOLED();
    printToOLED("WIFISETUP");
    wifiSetup();
    printToOLED("DASHSETUP");
    webserial();
    setupLoRaTransmitter();
    String ipAddress = WiFi.localIP().toString();
    printToOLED("IP: " + ipAddress);
    delay(1000);

    pinMode(TARE_BUTTON_PIN, INPUT_PULLUP);
    pinMode(FactoryReset_PIN, INPUT_PULLUP);
    pinMode(button3, INPUT_PULLUP);
    pinMode(activate_monitor, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TARE_BUTTON_PIN), tareButtonISR, FALLING); // Attach the interrupt
    attachInterrupt(digitalPinToInterrupt(FactoryReset_PIN), factoryResetISR, FALLING); // Attach the interrupt
    attachInterrupt(digitalPinToInterrupt(activate_monitor), activate_monitorISR, FALLING); // Attach the interrupt

    Serial.println("Starting BLE");
    Serial.println("Starting Serial");
    
    Serial.println("Starting OLED");
    
    pref.begin("beehive", false);
    newSetup = pref.getBool("newSetup"); // Get the newSetup flag from Preferences
    pref.end();

    checkForUpdates();
    loadPreferences();
    nauSetup();      // Try to initialize NAU7802
    attachcallbacks();
    // Optional: Print source
    if (nauAvailable) {
      Serial.println("Using NAU7802");
    } else if(!nauAvailable){
         myScale.powerUp(); 
         delay(1000);
         if(!nauAvailable){
            Serial.println("NAU7802 not detected. Using HX711.");
            WebSerial.println("NAU7802 not detected. Using HX711.");
            initScale();
         }
    } else{
      Serial.println("Using HX711");
      initScale();
    }
    
  


    if (reversedloadcell == 1) {
        WebSerial.println("******Reversed Load Cell*******");
        reverseloadcell();
    } 


    WebSerial.println("Checking Battery Level");
    Serial.println("Checking Battery Level");
    if (disablesleep == false) {
        if (battery > 3.5 || battery == 0) {
            WebSerial.println("Starting Wifi Setup");
            Serial.println("Starting Wifi Setup");
            initMQTT();
            WebSerial.println("Battery is above 3.5V. Entering Loop State.");
        } else if (battery < 3.5 && battery > 0) {
            enterDeepSleep(3600);
        }
    }
    disablesleep = false;
}

void loop() {
    WebSerial.println("/*************************************/");
    NWLoop();
    WebSerial.loop();
    if (WiFi.status() == WL_CONNECTED) {
        if (!mqttClient.connected()) {
            checkForUpdates();
            connectToMQTT();
            //mqttClient.publish((topicBase + "/backend/sleepmode").c_str(), String("Awake").c_str()); delay(100);
        }
        mqttClient.loop();

        uint8_t mac[6];
        WiFi.macAddress(mac);
        char macStr[13];
        if (Name != NULL) {
            strncpy(macStr, Name.c_str(), sizeof(macStr) - 1);
            macStr[sizeof(macStr) - 1] = '\0'; // Ensure null-termination
        } else {
            snprintf(macStr, sizeof(macStr), "%02X%02X%02X%02X", mac[2], mac[3], mac[4], mac[5]);
        }
    String topicBase = "beehive/data/";
    topicBase += macStr; // Get the last 4 digits of the MAC address


    if (tareRequested) {
        tareRequested = false; // Clear the flag
        tareDisplay(); // Call the tare function
        Serial.println("Tare Button Pressed");
        if (nauAvailable) {
            Serial.println("Taring NAU7802...");
            WebSerial.println("Taring NAU7802...");
            nauTare(10); // Tare NAU7802
        } else if(!nauAvailable) {
            Serial.println("Taring HX711...");
            WebSerial.println("Taring HX711...");
            tareScale(); // Tare HX711
        }
    }


    if(nauCalRequested) {
        nauCalibrate(1000, 100); // Calibrate NAU7802
        nauCalRequested = false; // Clear the flag
    }

    readDHTSensors();
    updateEXTTemp(temp1);
    updateEXTHum(h1);

    if (nauAvailable) {
        WebSerial.println("Reading NAU7802...");
        grams = nauRead(15); // Get the weight from NAU7802
    } else if(!nauAvailable) {
        WebSerial.println("Reading HX711...");
        updateScale();
    }
    
    updatezerocard(myScale.getZeroOffset()); // Function to update zero offset
    updatecalcard(myScale.getCalibrationFactor()); // Function to update calibration va

    float weightCorrected = grams + (t1 - T_BASELINE) * TEMP_SENSITIVITY;
    updateweightcard(grams);

    measureBattery();
    if (battery > oldbattery) {
        oldbattery = battery;
        disablesleep = true;
        charging = true;
    } else if (battery < oldbattery) {
        oldbattery = battery;
        disablesleep = false;
        charging = false;
    }
    updateOLED();

    pref.begin("beehive",false);
  
    float testvalue2 = pref.getFloat("calFactor", 0.0);
    



        int32_t zero = pref.getInt("zeroOffset",0);
        int cellconfig = pref.getInt("loadcellconfig",0);
        static float lastWeight = 0.0;
        static unsigned long lastPublishTime = 0;
        const unsigned long publishInterval = 15 * 60 * 1000; // 15 minutes in milliseconds

        if (abs(grams - lastWeight) > 1000 || millis() - lastPublishTime > publishInterval || heartbeat > 30) {
            Serial.println(abs(grams - lastWeight));
            mqttClient.publish((topicBase + "/temperature1").c_str(), String(temp1).c_str()); delay(100);
            mqttClient.publish((topicBase + "/humidity1").c_str(), String(h1).c_str()); delay(100);
            if(t2>0){mqttClient.publish((topicBase + "/temperature2").c_str(), String(t2).c_str()); delay(100);}
            if(h2>0){mqttClient.publish((topicBase + "/humidity2").c_str(), String(h2).c_str()); delay(100);}
            mqttClient.publish((topicBase + "/weight").c_str(), String(grams).c_str()); delay(100);
            mqttClient.publish((topicBase + "/tempadjustedweight").c_str(), String(weightCorrected).c_str()); delay(100);
            mqttClient.publish((topicBase + "/battery").c_str(), String(voltageDividerReading).c_str()); delay(100);
            mqttClient.publish((topicBase + "/backend/version").c_str(), String(currentVersion).c_str()); delay(100);
            mqttClient.publish((topicBase + "/lbs").c_str(), String(weightInPounds).c_str()); delay(100);
            mqttClient.publish((topicBase + "/backend/IP").c_str(), WiFi.localIP().toString().c_str()); delay(100);
            mqttClient.publish((topicBase + "/charging").c_str(), String(charging).c_str()); delay(100);
            mqttClient.publish((topicBase + "/backend/NAU7802").c_str(), String(nauAvailable).c_str()); delay(100);
            mqttClient.publish((topicBase + "/backend/CalValue").c_str(), String(testvalue2).c_str()); delay(100);
            mqttClient.publish((topicBase + "/backend/ZeroOffset").c_str(), String(zero).c_str()); delay(100);
            mqttClient.publish((topicBase + "/backend/setWeight").c_str(), String(grams).c_str()); delay(100);
            mqttClient.publish((topicBase + "/backend/loadcellconfig").c_str(), String(cellconfig).c_str()); delay(100);
    
            lastWeight = grams;
            lastPublishTime = millis();
            heartbeat = 0; // Reset heartbeat counter after publishing data
        } else {
            // Publish heartbeat
            Serial.println("Heartbeat sent to MQTT");
            WebSerial.println("Heartbeat sent to MQTT");
            mqttClient.publish((topicBase + "/heartbeat").c_str(), String(heartbeat).c_str()); delay(100);
            heartbeat++;
        }

        lastPublishTime = millis(); // Update the last publish time
        Serial.println("///////////////////LOOP///////////////////");
        Serial.println(topicBase);
        Serial.println("Published data to MQTT:");
        Serial.println("temperature1: " + String(temp1));
        Serial.println("Humidity1: " + String(h1));
        Serial.println("Temperature2: " + String(t2));
        Serial.println("Humidity2: " + String(h2));
        Serial.println("Weight: " + String(grams));
        Serial.println("mva: " + String(mVA));
        Serial.println("lbs: " + String(weightInPounds));
        Serial.println("///////////////UTIL//////////////////////");
        Serial.println("version: " + String(currentVersion));
        Serial.println("Battery: " + String(voltageDividerReading));
        Serial.println("Charging: " + String(charging));
        Serial.println("//////////////////////////////////////////");
        WebSerial.println("///////////////////LOOP///////////////////");
        WebSerial.println(topicBase);
        WebSerial.println("Published data to MQTT:");
        WebSerial.println("temperature1: " + String(temp1));
        WebSerial.println("Humidity1: " + String(h1));
        WebSerial.println("Temperature2: " + String(t2));
        WebSerial.println("Humidity2: " + String(h2));
        WebSerial.println("Weight: " + String(grams));
        WebSerial.println("mva: " + String(mVA));
        WebSerial.println("lbs: " + String(weightInPounds));
        WebSerial.println("///////////////UTIL//////////////////////");
        WebSerial.println("version: " + String(currentVersion));
        WebSerial.println("Battery: " + String(voltageDividerReading));
        WebSerial.println("Charging: " + String(charging));
        WebSerial.println("//////////////////////////////////////////");
    }
    updateOLEDWithNetworkStatus();


    WebSerial.println(String("Disable Sleep: ") + disablesleep);
    Serial.println(String("Disable Sleep: ") + disablesleep);

    pref.begin("beehive", false);
    int weighttest = pref.getInt("Weight", 0);
    WebSerial.println(String("Weight Test:  ") + weight);
    pref.putString("name", Name);
    pref.end();

    if (tareRequested) {
        tareRequested = false; // Clear the flag
        tareDisplay(); // Call the tare function
        Serial.println("Tare Button Pressed");
        tareScale(); // Call the tare function
    }
    timesincelastrestart++;
    Serial.println("Time Since Last Restart: " + String(timesincelastrestart));
    WebSerial.println("Time Since Last Restart: " + String(timesincelastrestart));
    if (timesincelastrestart > 5 && disablesleep == false) {
        timesincelastrestart = 0;
        ESP.restart();
    }else if (timesincelastrestart > 20 && disablesleep == true) {
        timesincelastrestart = 0;
        ESP.restart();
    }
    delay(1);

    if (disablesleep == false) {
        if (battery > 4.15) {
            WebSerial.println("Battery is above 4.15V. Restarting Loop.");
            Serial.println("Battery is above 4.15V. Restarting Loop.");
          //  mqttClient.publish((topicBase + "/backend/mode").c_str(), String("Loop").c_str()); delay(100);
        } else if (battery < 4.15 && battery > 3.9) {
            WebSerial.println("Battery is between 4.15V and 3.7V. Entering Light Sleep For 30 Min.");
            Serial.println("Battery is between 4.15V and 3.7V. Entering Light Sleep For 30 Min.");
          //  mqttClient.publish((topicBase + "/backend/mode").c_str(), String("30Min Sleep").c_str()); delay(100);
            myScale.powerDown();
            delay(1000);
            enterLightSleep(1800); // 30 minutes
        } else if (battery < 3.9 && battery > 3.7) {
            clearOLED();
            myScale.powerDown();
            WebSerial.println("Battery is below 3.7V. Entering Light Sleep for 1 Hour.");
            Serial.println("Battery is below 3.7V. Entering Light Sleep for 1 Hour.");
          //  mqttClient.publish((topicBase + "/backend/mode").c_str(), String("1 Hour Sleep").c_str()); delay(100);
            enterLightSleep(3600); // 1 hour
        } else {
            clearOLED();
            WebSerial.println("Deep Sleep for 3 Hours.");
            Serial.println("Battery is below 3.5V. Entering Deep Sleep for 2 Hour.");
           // mqttClient.publish((topicBase + "/backend/mode").c_str(), String("3 Hour Sleep").c_str()); delay(100);
            myScale.powerDown();
            delay(1000);
            enterLightSleep(10800); // 3 hours
        }
    } else {
        // Do nothing
    }
    printPreferences();
    //mqttClient.publish((topicBase + "/backend/sleepmode").c_str(), String("Light Sleep").c_str()); delay(100);
    printToOLED("Light Sleep Starting");
    delay(1000);
    printToOLED("...");
    enterLightSleep(30);
}

