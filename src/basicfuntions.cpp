#include <Arduino.h>
#include <Preferences.h>
#include "variables.h"



Preferences prefs;


void initSerial() {  
      Serial.begin(9600);
      while (!Serial) ; // Wait for Serial if needed
    
  }


  void loadPreferences() {
    prefs.begin("beehive"),false;
    last_weightstore = prefs.getInt("Weight");
    mVA              = prefs.getFloat("mVA");
    wifiSSID         = prefs.getString("wifiSSID", "");
    wifiPassword     = prefs.getString("wifiPassword", "");
    calibrationValue = prefs.getFloat("calibrationFactor", CALIBRATION_FACTOR);
  
    prefs.end();
  
  

      Serial.println(String("Last Weight=") + last_weightstore);
      Serial.println(String("Last mVA=") + mVA);
      Serial.println(String("Stored SSID=") + wifiSSID);
      Serial.println(String("Stored Password=") + wifiPassword);
      Serial.println(String("Stored Calibration=") + calibrationValue);
    
  }
  
  void clearPreferences() {
    prefs.begin("beehive",false);
    prefs.putInt("Weight",0);
    prefs.putFloat("mVA",0.0);
    last_weightstore = prefs.getInt("Weight");
    mVA              = prefs.getFloat("mVA");
    Serial.println(String("Last Weight=") + last_weightstore);
    Serial.println(String("Last mVA=") + mVA);
    Serial.println("Preferences Cleared");
    prefs.end();
  }
  