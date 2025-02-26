#include <Arduino.h>
#include <Preferences.h>
#include "variables.h"




Preferences prefs;

extern int mVA;
extern int last_weightstore;
extern float calibrationValue;


void initSerial() {  
      Serial.begin(9600);
      while (!Serial) ; // Wait for Serial if needed
    
  }


  void loadPreferences() {
    prefs.begin("beehive"),false;
    last_weightstore = prefs.getInt("Weight");
    mVA              = prefs.getFloat("mVA");
    calibrationValue = prefs.getFloat("calibrationFactor", CALIBRATION_FACTOR);
  
    prefs.end();
  
  

      Serial.println(String("Last Weight=") + last_weightstore);
      Serial.println(String("Last mVA=") + mVA);

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



  
void enterDeepSleep() {
  if(debug) {
    Serial.println("Entering deep sleep for 30  minutes...");
  }
  // Convert time from minutes to microseconds
  int timeInMicroseconds = 30 * 60 * 1000000;
  // Configure the wake-up source as a timer
  esp_sleep_enable_timer_wakeup(timeInMicroseconds);
  delay(1000);
  // Enter deep sleep mode
  esp_deep_sleep_start();
  delay(1000);
}

void enterNap() {
  if(debug) {
    Serial.println("Entering deep sleep for 1 minutes...");
  }
  // Convert time from minutes to microseconds
  int timeInMicroseconds = 1 * 60 * 1000000;
  // Configure the wake-up source as a timer
  esp_sleep_enable_timer_wakeup(timeInMicroseconds);
  delay(1000);
  // Enter deep sleep mode
  esp_deep_sleep_start();
  delay(1000);
}
  