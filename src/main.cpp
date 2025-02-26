
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

/////////////////

float battery = 0.0;
float grams = 0.0;
float temp1 = NAN;
float temp2 = NAN;
float weight = 0.0;
int counter = 0;
int mVA;
bool weightset;




void setup() {
    initSerial();
    wmsetup();

}



void loop() {

}

