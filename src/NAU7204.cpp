#include <Wire.h>
#include <Preferences.h>
#include "NAU7204.h"
#include "webserialsetup.h"
#include "OLED.h"
#include "variables.h"

extern Adafruit_SSD1306 display;

NAU7802 myScale;
extern Preferences prefs;

bool scaleReady = false;

#define PREF_NAMESPACE "beehive"
#define PREF_KEY_SCALE "calFactor"
#define PREF_KEY_ZERO  "zeroOffset"

#define AVG_SIZE 4
float avgWeights[AVG_SIZE];
uint8_t avgWeightSpot = 0;


//myScale.powerDown(); //Power down to ~200nA
//delay(1000);
//myScale.powerUp(); //Power up scale. This scale takes ~600ms to boot and take reading.
//myScale.setGain(NAU7802_GAIN_2); //Gain can be set to 1, 2, 4, 8, 16, 32, 64, or 128.
//myScale.setSampleRate(NAU7802_SPS_40); //Sample rate can be set to 10, 20, 40, 80, or 320Hz
//myScale.setLDOVoltage(NAU7802_LDO_3V); //LDO voltage can be set to 2.7, 3.0, or 3.3V


bool nauSetup() {
  Wire.begin();
  Wire.setClock(400000);

  if (!myScale.begin()) {
    Serial.println("NAU7802 not detected.");
    return false;
  }

  Serial.println("NAU7802 detected.");

  readScaleSettings();

  if (last_weightstore > 0) {
    Serial.println("Last weight store is greater than 0. Taring the scale...");
    myScale.calculateZeroOffset(10); // Tare the scale with 10 samples
    myScale.setZeroOffset(last_weightstore); // Apply the last weight as the zero offset
    Serial.print("Applied last weight store as zero offset: ");
    Serial.println(last_weightstore);
  } 

  prefs.begin("beehive", false);
  float oldcalvalue = myScale.getCalibrationFactor();
  Serial.print("Old calibration factor: ");
  Serial.println(oldcalvalue);
  float prefscalvalue = prefs.getFloat("calFactor", 0.0);
  Serial.print("Pref calibration factor: ");
  Serial.println(prefscalvalue);
  
  int storedoffset = prefs.getInt("zeroOffset", myScale.getZeroOffset());
  prefs.end();

  myScale.setZeroOffset(storedoffset);
  WebSerial.print("Stored zero offset: ");
  WebSerial.println(storedoffset);

  myScale.setGain(NAU7802_GAIN_128);
  myScale.setSampleRate(NAU7802_SPS_320);
  myScale.setLDO(NAU7802_LDO_3V3);
  myScale.calibrateAFE();

  Serial.print("Zero offset: ");
  Serial.println(myScale.getZeroOffset());
  Serial.print("Calibration factor: ");
  Serial.println(myScale.getCalibrationFactor());
  nauAvailable = true; // Set nauAvailable to true if NAU7802 is detected
  scaleReady = true;
  return true;
}

void nauTare(uint16_t samples) {
  if (!scaleReady) return;
  myScale.calculateZeroOffset(samples);
}

void nauCalibrate(float knownWeight, uint16_t samples) {
  if (!scaleReady) return;

  WebSerial.println("Taring before calibration...");
  myScale.calculateZeroOffset(samples);
  WebSerial.println("Put Weight on scale.");
  delay(15000); // Wait for user to place weight
  Serial.print("Raw value read from NAU7802: ");
  WebSerial.println("Reading weight...Now");
  delay(1000); // Wait for reading to stabilize
  WebSerial.println("Calibrating...");
  myScale.calculateCalibrationFactor(knownWeight, samples);

  WebSerial.print("New calibration factor: ");
  WebSerial.println(myScale.getCalibrationFactor());

  recordScaleSettings();
}

float nauRead(uint8_t sampleCount) {



  Serial.println("Reading NAU7802 in Loop...");
  WebSerial.println("Reading NAU7802...");


  float total = 0;
  float totalraw = 0;
  int valid = 0;

  for (int i = 0; i < sampleCount; i++) {
    
      float reading = myScale.getWeight();
      //if (reading < 10000 && reading > -10000) {
        Serial.print("Reading Scale:  ");
        Serial.println(myScale.getReading());
        WebSerial.print("Reading Scale:  ");
        WebSerial.println(myScale.getReading());
        display.clearDisplay();
        display.setTextSize(1); // Set text size to 2 for larger text
        display.setTextColor(SSD1306_WHITE); // Set text color to white
        display.setCursor(0, 0); // Set cursor to top-left corner
        display.print("Reading: ");
        display.setTextSize(2); // Set text size to 2 for larger text
        display.setCursor(0, 15); // Set cursor to top-left corner
        display.print(myScale.getWeight());
        display.display(); // Display the text
    
        
          
       
        total += reading;
      //  valid++;
    //  }
    
    delay(10);
  }
  for (int i = 0; i < sampleCount; i++) {
    
      float readingraw = myScale.getReading();
      
        
        totalraw += readingraw;
        
      
    
    delay(10);
  }

 
  float weightraw = totalraw/sampleCount;
  float weight = total / sampleCount;

  prefs.begin("beehive", false);
  int loadcellconfig = prefs.getInt("loadcellconfig", 0);
  prefs.end();
  if (loadcellconfig == 1) {
    weight = weight*2; // One Side load cell configuration
  }else if (loadcellconfig == 2) {
    weight = weight; // Two Side load cell configuration
  }else {

    weight = weight; // Default load cell configuration
  }

  WebSerial.println(String("Grams: ") + weight);
  WebSerial.println(String("RAW: ") + weightraw);
  // Convert grams to pounds (1 gram = 0.00220462 pounds)
  weightInPounds = weight * 0.00220462;
  WebSerial.println(String("Weight in Pounds: ") + weightInPounds);
  prefs.begin("beehive", false);
  prefs.putInt("Weight", weight);
  prefs.end();

  return weight;
}

void recordScaleSettings() {
  prefs.begin(PREF_NAMESPACE, false);
  prefs.putFloat("calFactor", myScale.getCalibrationFactor());
  prefs.putInt(PREF_KEY_ZERO, myScale.getZeroOffset());
  prefs.end();
}

void readScaleSettings() {
  prefs.begin("beehive", false);


    float scale = prefs.getFloat("calFactor", 0.0); 
    myScale.setCalibrationFactor(scale);
    Serial.print("Scale factor: ");
    Serial.println(scale);

    int32_t zero = prefs.getInt(PREF_KEY_ZERO);
    myScale.setZeroOffset(zero);
    Serial.print("Zero offset: ");
    Serial.println(zero);


  prefs.end();
}
