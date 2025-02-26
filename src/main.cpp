
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

/////////////////

float battery = 0.0;
float grams = 0.0;
float temp1 = NAN;
float temp2 = NAN;
float weight = 0.0;
int counter = 0;
float mVA;
bool weightset;




void setup() {
    initSerial();
    wmsetup();
    loadPreferences();
    initDHTSensors();
    initScale();
    webserial();
    WebSerial.println("Starting Hive Monitor...");
    
}



void loop() {

    readDHTSensors();
    updateScale();
    measureBattery();
    checkForUpdates();
    delay(1000);    
    WebSerial.println("//////////////////////////////////////////");delay(100);
    WebSerial.println("Battery: " + String(battery));delay(100);
    WebSerial.println("Weight: " + String(weightInPounds));delay(100);
    WebSerial.println("Temp1: " + String(temp1));delay(100);
    WebSerial.println("Temp2: " + String(temp2));delay(100);
    WebSerial.println("Humidity1: " + String(h1));delay(100);
    WebSerial.println("Humidity2: " + String(h2));delay(100);
    WebSerial.println("//////////////////////////////////////////");delay(100);
    WebSerial.loop();
    
}

