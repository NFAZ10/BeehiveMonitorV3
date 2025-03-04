#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

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
/////////////////

float battery = 0.0;
float grams = 0.0;
float temp1 = NAN;
float temp2 = NAN;
float weight = 0.0;
int counter = 0;
float mVA;
bool weightset = false;


Preferences pref;

void IRAM_ATTR tareButtonISR() {
    tareRequested = true; // Set the flag when the interrupt is triggered
}

void setup() {
    pinMode(TARE_BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TARE_BUTTON_PIN), tareButtonISR, FALLING); // Attach the interrupt

    strip.begin();
    strip.show(); // Turn OFF all pixels ASAP
    strip.setBrightness(50);

    initSerial();
    
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
            strip.setPixelColor(0,100,0,0);
            strip.show();
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
        Serial.println("Tare Button Pressed");
        tareScale(); // Call the tare function
    }

    readDHTSensors();
    updateScale();
    measureBattery();
    checkForUpdates();

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
        strip.setPixelColor(0,0,255,0); // Set pixel's color (in RAM)
        strip.show();
        mqttClient.loop();

        uint8_t mac[6];
        WiFi.macAddress(mac);
        char macStr[13];
        snprintf(macStr, sizeof(macStr), "%02X%02X%02X%02X", mac[2], mac[3], mac[4], mac[5]);
        String topicBase = "beehive/data/";
        topicBase += macStr; // Get the last 4 digits of the MAC address

        mqttClient.publish((topicBase + "/temperature1").c_str(), String(t1).c_str()); delay(1000);
        mqttClient.publish((topicBase + "/humidity1").c_str(), String(h1).c_str()); delay(1000);
        mqttClient.publish((topicBase + "/temperature2").c_str(), String(t2).c_str()); delay(1000);
        mqttClient.publish((topicBase + "/humidity2").c_str(), String(h2).c_str()); delay(1000);
        mqttClient.publish((topicBase + "/weight").c_str(), String(grams).c_str()); delay(1000);
        mqttClient.publish((topicBase + "/battery").c_str(), String(voltageDividerReading).c_str()); delay(1000);
        mqttClient.publish((topicBase + "/version").c_str(), String(currentVersion).c_str()); delay(1000);
        mqttClient.publish((topicBase + "/mva").c_str(), String(mVA).c_str()); delay(1000);
        mqttClient.publish((topicBase + "/lbs").c_str(), String(weightInPounds).c_str()); delay(1000);
        mqttClient.publish((topicBase + "/IP").c_str(), WiFi.localIP().toString().c_str()); delay(1000);

        lastPublishTime = millis(); // Update the last publish time
        Serial.println("///////////////////LOOP///////////////////");
        Serial.println(topicBase);
        Serial.println("Published data to MQTT:");
        Serial.println("temperature1: " + String(t1));
        Serial.println("Humidity1: " + String(h1));
        Serial.println("Temperature2: " + String(t2));
        Serial.println("Humidity2: " + String(h2));
        Serial.println("Weight: " + String(grams));
        Serial.println("mva: " + String(mVA));
        Serial.println("lbs: " + String(weightInPounds));
        Serial.println("///////////////UTIL//////////////////////");
        Serial.println("version: " + String(currentVersion));
        Serial.println("Battery: " + String(voltageDividerReading));
        Serial.println("//////////////////////////////////////////");
    }

    WebSerial.println(String("Disable Sleep: ") + disablesleep);
    Serial.println(String("Disable Sleep: ") + disablesleep);

    pref.begin("beehive", false);
    int weighttest = pref.getInt("Weight", 0);
    WebSerial.println(String("Weight Test:  ") + weight);
    pref.end();

    if (disablesleep == false) {
        if (battery > 4.15) {
            WebSerial.println("Battery is above 4.15V. Restarting Loop.");
            Serial.println("Battery is above 4.15V. Restarting Loop.");
            strip.setPixelColor(0,0,255,0); strip.show();
        } else if (battery < 4.15 && battery > 3.7) {
            WebSerial.println("Battery is between 4.15V and 3.7V. Entering Light Sleep For 30 Min.");
            Serial.println("Battery is between 4.15V and 3.7V. Entering Light Sleep For 30 Min.");
            strip.setPixelColor(0,0,0,75); strip.show();
            delay(1000);
            enterLightSleep(1800);
        } else if (battery < 3.7 && battery > 3.5) {
            WebSerial.println("Battery is below 3.7V. Entering Light Sleep for 1 Hour.");
            Serial.println("Battery is below 3.7V. Entering Light Sleep for 1 Hour.");
            strip.setPixelColor(0,0,0,255); strip.show();
            delay(1000);
            strip.setPixelColor(0,0,0,0); strip.show();
            delay(1000);
            enterLightSleep(3600);
        } else {
            WebSerial.println("Battery is below 3.5V. Entering Deep Sleep for 2 Hour.");
            Serial.println("Battery is below 3.5V. Entering Deep Sleep for 2 Hour.");
            strip.setPixelColor(0,255,0,0); strip.show();
            delay(1000);
            strip.setPixelColor(0,0,0,0); strip.show();
            delay(1000);
            delay(1000);
            enterDeepSleep(7200);
        }
    } else {
        strip.setPixelColor(0,0,0,0);
        strip.show();
    }
    delay(1000);
}

