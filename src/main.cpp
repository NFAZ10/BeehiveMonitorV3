#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "esp_task_wdt.h"

/////////////////
///Our Setups////
/////////////////

#include "wifisetup.h"
#include "webserialsetup.h"
#include "basicfunctions.h"
#include "sensors.h"
#include "variables.h"
#include "ota.h"
#include "mqtt.h"
#include <Preferences.h>
#include "OLED.h"
//#include "ble.h"

#define DEBUG 0
#if DEBUG
  #define DEBUG_PRINT(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
#endif



#define WDT_TIMEOUT 50


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

Preferences pref;

void IRAM_ATTR tareButtonISR() {
    tareRequested = true; // Set the flag when the interrupt is triggered
}

void setup() {
    initSerial();
    esp_task_wdt_init(WDT_TIMEOUT,true);
    esp_task_wdt_add(NULL);
   
    pinMode(TARE_BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TARE_BUTTON_PIN), tareButtonISR, FALLING); // Attach the interrupt


    //BLEsetup();
    Serial.println("Starting BLE");
    Serial.println("Starting Serial");

    setupOLED();
    Serial.println("Starting OLED");
    loadPreferences();
    initDHTSensors();
    initScale();
    measureBattery();
    WebSerial.println("Checking Battery Level");
    Serial.println("Checking Battery Level");
    if(disablesleep == false){
        if(battery > 3.5 || battery == 0){
            WebSerial.println("Starting Wifi Setup");
            Serial.println("Starting Wifi Setup");

            wmsetup();
            webserial();
            initMQTT();

            WebSerial.println("Battery is above 3.5V. Entering Loop State.");

        } else if (battery < 3.5 && battery > 0) {
            enterDeepSleep(3600);
        }
    }
}

void loop() {
    WebSerial.loop();


    if (tareRequested) {
        tareRequested = false; // Clear the flag
        tareDisplay(); // Call the tare function
        Serial.println("Tare Button Pressed");
        tareScale(); // Call the tare function
    }

    readDHTSensors();
    updateScale();
    measureBattery();
    if (battery>oldbattery){
    oldbattery=battery;
    disablesleep=true;
    charging=true;

    }else if (battery<oldbattery){
    oldbattery=battery;
    disablesleep=false;
    charging=false;
    }

    checkForUpdates();
    updateOLED();

/*    if (doConnect == true) {
        if (connectToServer()) {
          Serial.println("We are now connected to the BLE Server.");
        } else {
          Serial.println("We have failed to connect to the server; there is nothing more we will do.");
        }
        doConnect = false;
      }
    
      // If we are connected to a peer BLE Server, update the characteristic each time we are reached
      // with the current time since boot.
      if (connected) {
        String newValue = "Time since boot: " + String(millis() / 1000);
        Serial.println("Setting new characteristic value to \"" + newValue + "\"");
    
        // Set the characteristic's value to be the array of bytes that is actually a string.
        pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
      } else if (doScan) {
        BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
      }

    */
    WebSerial.println("//////////////////LOOP////////////////"); delay(1000);
    WebSerial.println("Battery: " + String(battery)); delay(1000);
    WebSerial.println("Weight: " + String(weightInPounds)); delay(1000);
    WebSerial.println("Temp1: " + String(temp1)); delay(1000);
    WebSerial.println("Temp2: " + String(temp2)); delay(1000);
    WebSerial.println("Humidity1: " + String(h1)); delay(1000);
    WebSerial.println("Humidity2: " + String(h2)); delay(1000);
    WebSerial.println("Version: " + String(currentVersion)); delay(1000);
    WebSerial.println("//////////////////////////////////////////"); delay(1000);
    WebSerial.println(String("Tare Status: ") + tareRequested); delay(1000);

    if (WiFi.status() == WL_CONNECTED) {
        if (!mqttClient.connected()) {
            checkForUpdates();
            connectToMQTT();
        }
;
        mqttClient.loop();

        uint8_t mac[6];
        WiFi.macAddress(mac);
        char macStr[13];
        snprintf(macStr, sizeof(macStr), "%02X%02X%02X%02X", mac[2], mac[3], mac[4], mac[5]);
        String topicBase = "beehive/data/";
        topicBase += macStr; // Get the last 4 digits of the MAC address

        mqttClient.publish((topicBase + "/temperature1").c_str(), String(temp1).c_str()); delay(1000);
        mqttClient.publish((topicBase + "/humidity1").c_str(), String(h1).c_str()); delay(1000);
        mqttClient.publish((topicBase + "/temperature2").c_str(), String(t2).c_str()); delay(1000);
        mqttClient.publish((topicBase + "/humidity2").c_str(), String(h2).c_str()); delay(1000);
        mqttClient.publish((topicBase + "/weight").c_str(), String(grams).c_str()); delay(1000);
        mqttClient.publish((topicBase + "/battery").c_str(), String(voltageDividerReading).c_str()); delay(1000);
        mqttClient.publish((topicBase + "/version").c_str(), String(currentVersion).c_str()); delay(1000);
        mqttClient.publish((topicBase + "/mva").c_str(), String(mVA).c_str()); delay(1000);
        mqttClient.publish((topicBase + "/lbs").c_str(), String(weightInPounds).c_str()); delay(1000);
        mqttClient.publish((topicBase + "/IP").c_str(), WiFi.localIP().toString().c_str()); delay(1000);
        mqttClient.publish((topicBase + "/charging").c_str(), String(charging).c_str()); delay(1000);

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
        esp_task_wdt_reset();
    }
    updateOLEDWithNetworkStatus();
    WebSerial.println(String("Disable Sleep: ") + disablesleep);
    Serial.println(String("Disable Sleep: ") + disablesleep);

    pref.begin("beehive", false);
    int weighttest = pref.getInt("Weight", 0);
    WebSerial.println(String("Weight Test:  ") + weight);
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
    if (timesincelastrestart > 5) {
        timesincelastrestart = 0;
        ESP.restart();
    }
    delay(5000);
    clearOLED();
    esp_task_wdt_reset();
    if (disablesleep == false) {
        if (battery > 4.15) {
            //printToOLED("Restarting Loop.");
            WebSerial.println("Battery is above 4.15V. Restarting Loop.");
            Serial.println("Battery is above 4.15V. Restarting Loop.");
            
        } else if (battery < 4.15 && battery > 3.7) {
            //printToOLED("BLight Sleep For 30 Min.");
            WebSerial.println("Battery is between 4.15V and 3.7V. Entering Light Sleep For 30 Min.");
            Serial.println("Battery is between 4.15V and 3.7V. Entering Light Sleep For 30 Min.");
     
            delay(1000);
            enterLightSleep(1800);
        } else if (battery < 3.7 && battery > 3.5) {
            //printToOLED("Light Sleep for 1 Hour.");
            WebSerial.println("Battery is below 3.7V. Entering Light Sleep for 1 Hour.");
            Serial.println("Battery is below 3.7V. Entering Light Sleep for 1 Hour.");

            enterLightSleep(3600);
        } else {
            WebSerial.println("Deep Sleep for 2 Hour.");
            Serial.println("Battery is below 3.5V. Entering Deep Sleep for 2 Hour.");

            delay(1000);
            enterDeepSleep(7200);
        }
    } else {

    }
    delay(1000);
}

