#include <Arduino.h>
#include <Preferences.h>
#include "variables.h"
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "wifisetup.h"



Preferences prefs;
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

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



  

  #define HX711_CLK  5  // Change to your actual HX711 clock pin
  #define HX711_DATA 18  // Change to your actual HX711 data pin
  
  void enterDeepSleep(uint64_t sleepTimeSeconds) {
      Serial.println("Powering down HX711...");
      
      // Set HX711 into power-down mode (Clock HIGH)
      digitalWrite(HX711_CLK, HIGH);
      delayMicroseconds(100); // Ensure power-down command is sent
  
      Serial.println("Entering Deep Sleep...");
  
      // Convert seconds to microseconds
      uint64_t sleepTimeMicroseconds = sleepTimeSeconds * 1000000ULL;
  
      // Enable wake-up timer
      esp_sleep_enable_timer_wakeup(sleepTimeMicroseconds);
  
      // Go to sleep
      esp_deep_sleep_start();
  }

  void enterLightSleep(uint64_t sleepTimeSeconds) {
    Serial.println("Powering down HX711...");
    
    // Set HX711 into power-down mode (Clock HIGH)
    digitalWrite(HX711_CLK, HIGH);
    delayMicroseconds(100); // Ensure power-down command is sent

    Serial.println("Entering Light Sleep...");

    // Convert seconds to microseconds
    uint64_t sleepTimeMicroseconds = sleepTimeSeconds * 1000000ULL;

    // Enable wake-up timer
    esp_sleep_enable_timer_wakeup(sleepTimeMicroseconds);
    strip.setPixelColor(0,0,0,0); //  Set pixel's color (in RAM)
    strip.show();
    delay(1000);
    // Start light sleep
    esp_light_sleep_start();
    wmsetup();
    Serial.println("Woke up from Light Sleep");
    delay(2000);
}
  