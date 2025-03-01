
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

/////////////////

float battery = 0.0;
float grams = 0.0;
float temp1 = NAN;
float temp2 = NAN;
float weight = 0.0;
int counter = 0;
float mVA;
bool weightset = false;



void setup() {

    strip.begin();
    strip.show(); // Turn OFF all pixels ASAP
    strip.setBrightness(50);

    initSerial();
    
    loadPreferences();
    initDHTSensors();
    initScale();
  


    Serial.println("Starting Hive Monitor...");

    readDHTSensors();
    updateScale();
    measureBattery();
    checkForUpdates();
    delay(1000);    


    // Check for tare request
    if(tareRequested==true) {
        tareScale();
        tareRequested = false;
    }


    wmsetup();
    webserial();
    if(WiFi.status() == WL_CONNECTED){
            // Print the IP address
        Serial.print("IP Address: ");
         Serial.println(WiFi.localIP());
        initMQTT();
        connectToMQTT();
    }

    
    WebSerial.println("//////////////////////////////////////////");delay(100);
    WebSerial.println("Battery: " + String(battery));delay(100);
    WebSerial.println("Weight: " + String(weightInPounds));delay(100);
    WebSerial.println("Temp1: " + String(temp1));delay(100);
    WebSerial.println("Temp2: " + String(temp2));delay(100);
    WebSerial.println("Humidity1: " + String(h1));delay(100);
    WebSerial.println("Humidity2: " + String(h2));delay(100);
    WebSerial.println("//////////////////////////////////////////");delay(100);
    WebSerial.println(String("Tare Status: ") + tareRequested);delay(100);
    WebSerial.loop();


    if (WiFi.status() == WL_CONNECTED) {
        if (!mqttClient.connected()) {
      //connectToMQTT();
    }
    strip.setPixelColor(0,0,255,0); //  Set pixel's color (in RAM)
    strip.show();
      mqttClient.loop();
  
      
        uint8_t mac[6];
        WiFi.macAddress(mac);
        char macStr[13];
        snprintf(macStr, sizeof(macStr), "%02X%02X%02X%02X", mac[2], mac[3], mac[4], mac[5]);
        String topicBase = "beehive/data/";
        topicBase += macStr; // Get the last 4 digits of the MAC address
  
        mqttClient.publish((topicBase + "/temperature1").c_str(), String(t1).c_str());
        delay(1000);
        mqttClient.publish((topicBase + "/humidity1").c_str(), String(h1).c_str());
        delay(1000);
        mqttClient.publish((topicBase + "/temperature2").c_str(), String(t2).c_str());
        delay(1000);
        mqttClient.publish((topicBase + "/humidity2").c_str(), String(h2).c_str());
        delay(1000);
        mqttClient.publish((topicBase + "/weight").c_str(), String(grams).c_str());
        delay(1000);
        mqttClient.publish((topicBase + "/battery").c_str(), String(voltageDividerReading).c_str());
        delay(1000);
        mqttClient.publish((topicBase + "/version").c_str(), String(currentVersion).c_str());
        delay(1000);
        mqttClient.publish((topicBase + "/mva").c_str(), String(mVA).c_str());
        delay(1000);
        mqttClient.publish((topicBase + "/lbs").c_str(), String(weightInPounds).c_str());
        delay(1000);

  
        lastPublishTime = millis(); // Update the last publish time
        Serial.println(topicBase);
        if (debug) {
          Serial.println("Published data to MQTT:");
          Serial.println("temperature1: " + String(t1));
          Serial.println("humidity1: " + String(h1));
          Serial.println("temperature2: " + String(t2));
          Serial.println("humidity2: " + String(h2));
          Serial.println("weight: " + String(grams));
          Serial.println("battery: " + String(voltageDividerReading));
          Serial.println("version: " + String(currentVersion));
          Serial.println("mva: " + String(mVA));
          Serial.println("lbs: " + String(weightInPounds));

        }
      
    }
    strip.setPixelColor(0,10,0,0);
    strip.show(); //  strip.show(); // 
     if(disablesleep == false){
        if (battery > 4.0 || battery == 0) {
   //  strip.show(); // 
             enterNap();
         }else if(battery > 4.15){
            strip.setPixelColor(0,100,0,0);
            strip.show();
         }
         else {
             enterDeepSleep();
         }
        }
}

void loop(){
    readDHTSensors();
    updateScale();
    measureBattery();
    checkForUpdates();
    WebSerial.println("//////////////////////////////////////////");delay(100);
    WebSerial.println("Battery: " + String(battery));delay(100);
    WebSerial.println("Weight: " + String(weightInPounds));delay(100);
    WebSerial.println("Temp1: " + String(temp1));delay(100);
    WebSerial.println("Temp2: " + String(temp2));delay(100);
    WebSerial.println("Humidity1: " + String(h1));delay(100);
    WebSerial.println("Humidity2: " + String(h2));delay(100);
    WebSerial.println("//////////////////////////////////////////");delay(100);
    WebSerial.println(String("Tare Status: ") + tareRequested);delay(100);
    WebSerial.loop();

    if (WiFi.status() == WL_CONNECTED) {
        if (!mqttClient.connected()) {
      connectToMQTT();
    }
    strip.setPixelColor(0,0,255,0); //  Set pixel's color (in RAM)
    strip.show();
      mqttClient.loop();
  
      if (millis() - lastPublishTime >= publishInterval) {
        uint8_t mac[6];
        WiFi.macAddress(mac);
        char macStr[13];
        snprintf(macStr, sizeof(macStr), "%02X%02X%02X%02X", mac[2], mac[3], mac[4], mac[5]);
        String topicBase = "beehive/data/";
        topicBase += macStr; // Get the last 4 digits of the MAC address
  
        mqttClient.publish((topicBase + "/temperature1").c_str(), String(t1).c_str());
        delay(1000);
        mqttClient.publish((topicBase + "/humidity1").c_str(), String(h1).c_str());
        delay(1000);
        mqttClient.publish((topicBase + "/temperature2").c_str(), String(t2).c_str());
        delay(1000);
        mqttClient.publish((topicBase + "/humidity2").c_str(), String(h2).c_str());
        delay(1000);
        mqttClient.publish((topicBase + "/weight").c_str(), String(grams).c_str());
        delay(1000);
        mqttClient.publish((topicBase + "/battery").c_str(), String(voltageDividerReading).c_str());
        delay(1000);
        mqttClient.publish((topicBase + "/version").c_str(), String(currentVersion).c_str());
        delay(1000);
        mqttClient.publish((topicBase + "/mva").c_str(), String(mVA).c_str());
        delay(1000);
        mqttClient.publish((topicBase + "/lbs").c_str(), String(weightInPounds).c_str());
        delay(1000);

  
        lastPublishTime = millis(); // Update the last publish time
        Serial.println(topicBase);
     
          Serial.println("Published data to MQTT:");
          Serial.println("temperature1: " + String(t1));
          Serial.println("humidity1: " + String(h1));
          Serial.println("temperature2: " + String(t2));
          Serial.println("humidity2: " + String(h2));
          Serial.println("weight: " + String(grams));
          Serial.println("battery: " + String(voltageDividerReading));
          Serial.println("version: " + String(currentVersion));
          Serial.println("mva: " + String(mVA));
          Serial.println("lbs: " + String(weightInPounds));

        
      }
    }

    delay(60000);   

}