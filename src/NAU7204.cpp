#include <Wire.h>
#include <Preferences.h>
#include "NAU7204.h"
#include "webserialsetup.h"
#include "OLED.h"
#include "variables.h"

NAU7802 myScale;
extern Preferences prefs;

bool scaleReady = false;

#define PREF_NAMESPACE "beehive"
#define PREF_KEY_SCALE "calFactor"
#define PREF_KEY_ZERO  "zeroOffset"

#define AVG_SIZE 4
float avgWeights[AVG_SIZE];
uint8_t avgWeightSpot = 0;

bool nauSetup() {
  Wire.begin();
  Wire.setClock(400000);

  if (!myScale.begin()) {
    Serial.println("NAU7802 not detected.");
    return false;
  }

  Serial.println("NAU7802 detected.");

  readScaleSettings();

  myScale.setSampleRate(NAU7802_SPS_320);
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
  if (!scaleReady) return -999;

  float total = 0;
  int valid = 0;

  for (int i = 0; i < sampleCount; i++) {
    if (myScale.available()) {
      float reading = myScale.getWeight();
      if (reading < 10000 && reading > -10000) {
        
          Serial.print("Reading: ");
          Serial.println(reading);
          
         
        
        total += reading;
        valid++;
      }
    }
    delay(10);
  }

  if (valid == 0) return -999;

  float weight = total / valid;

  avgWeights[avgWeightSpot++] = weight;
  if (avgWeightSpot >= AVG_SIZE) avgWeightSpot = 0;

  float avgWeight = 0;
  for (int i = 0; i < AVG_SIZE; i++) {
    avgWeight += avgWeights[i];
  }

  return avgWeight / AVG_SIZE;
}

void recordScaleSettings() {
  prefs.begin(PREF_NAMESPACE, false);
  prefs.putFloat(PREF_KEY_SCALE, myScale.getCalibrationFactor());
  prefs.putInt(PREF_KEY_ZERO, myScale.getZeroOffset());
  prefs.end();
}

void readScaleSettings() {
  prefs.begin(PREF_NAMESPACE, true);

  if (prefs.isKey(PREF_KEY_SCALE)) {
    float scale = prefs.getFloat(PREF_KEY_SCALE);
    myScale.setCalibrationFactor(scale);
  } else {
    myScale.setCalibrationFactor(1.0);
  }

  if (prefs.isKey(PREF_KEY_ZERO)) {
    int32_t zero = prefs.getInt(PREF_KEY_ZERO);
    myScale.setZeroOffset(zero);
  } else {
    myScale.setZeroOffset(0);
  }

  prefs.end();
}
