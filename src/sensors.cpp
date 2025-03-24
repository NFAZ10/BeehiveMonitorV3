#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include "HX711_ADC.h"
#include "DHT.h"
#include "ArduinoJson.h"

#include <Preferences.h>
#include <ArduinoJson.h>
#include "variables.h"
#include <esp_task_wdt.h>
#include "webserialsetup.h"
#include "OLED.h"



DHT dht1(27, DHT11);
DHT dht2(26, DHT11);
HX711_ADC LoadCell(5, 18);

extern float battery;
extern float grams;
extern float temp1;
extern float temp2;
extern float weight;
extern int counter;
extern float mVA;
extern bool weightset;
extern Preferences prefs;
extern int numberofloadcells;



void initDHTSensors() {
    dht1.begin();
    dht2.begin();
  }


  void tareScale() {
    // Start the tare process
    WebSerial.println("Tare started...");
  

    LoadCell.update();
  
     LoadCell.refreshDataSet();
     LoadCell.resetSamplesIndex();
     //LoadCell.setTareOffset(0);
     delay(1000);
  
    LoadCell.tareNoDelay();
    
    Serial.println("Tare started...");
    
  
    // Reset variables after taring
    grams  = 0;
    mVA    = 0;
    weight = 0;
    last_weightstore = 0;
    prefs.begin("beehive-data",false);
    prefs.putInt("lastWeight", grams);
    prefs.putInt("Weight", grams);
    prefs.putFloat("mVA", mVA);
    prefs.end();
  
  
  
      WebSerial.println("Tare completed:");
      WebSerial.println("Raw Grams: 0");
      WebSerial.println("Moving Avg Grams: 0");
      WebSerial.println("Weight (oz): 0");
    
  
  
  
    
      Serial.println("Rebooting after tare...");
    
    delay(500); // Allow Serial message to complete
    weightset=false;
    //ESP.restart(); // Reboot the ESP

  }
  

  void initScale() {

    if(debug) Serial.println("Initializing HX711...");
    LoadCell.begin();
    

    if (reversedloadcell==true) {
      WebSerial.println("******Reversed Load Cell*******");
      LoadCell.setReverseOutput();
    }
    else {

    }

  
    unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
    boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
    LoadCell.start(stabilizingtime, _tare);
    if (LoadCell.getTareTimeoutFlag()) {
      Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
      while (1);
    }
    else {
      if(mVA>0){
        if(mVA-last_weightstore>=10000){
          Serial.println("Negative Weight Detected. Taring...");
          tareScale();
        }
        mVA = mVA*-calibrationValue;
       
      }
      LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
  
      
      Serial.println("Startup is complete");
    }
  
  
  }



  
float movingAverage(int newValue) {
    static float readings[NUM_SAMPLES] = {0};
    static int index = 0;
    static float sum = 0;

    // Load readings from Preferences if it's the first time
    if (index == 0) {
        prefs.begin("movingAvg", false);
        for (int i = 0; i < NUM_SAMPLES; i++) {
            readings[i] = prefs.getFloat(("reading" + String(i)).c_str(), 0);
            sum += readings[i];
        }
        prefs.end();
    }

    // Subtract the oldest reading from sum
    sum -= readings[index];

    // Store the new reading
    readings[index] = newValue;
    sum += newValue;

    // Save the new reading to Preferences
    prefs.begin("movingAvg", false);
    prefs.putFloat(("reading" + String(index)).c_str(), newValue);
    prefs.end();

    // Advance the index, wrapping around
    index = (index + 1) % NUM_SAMPLES;

    // Return the average
    return sum / NUM_SAMPLES;
}
  

  
void readDHTSensors() {
  h1 = dht1.readHumidity();
  t1 = dht1.readTemperature();
  h2 = dht2.readHumidity();
  t2 = dht2.readTemperature();

  // Update cloud variables in Fahrenheit (as used in your code)
  temp1 = (t1 * 9.0f / 5.0f) + 32.0f;
  temp2 = (t2 * 9.0f / 5.0f) + 32.0f;
  // Check for NaN values and set to null if found
  if (isnan(h1)) h1 = -1;
  if (isnan(t1)) t1 = -1;
  if (isnan(h2)) h2 = -1;
  if (isnan(t2)) t2 = -1;
  // Debug printing
 
    if(t1 >= 0) {
      Serial.println(String("Temp1: ") + temp1 + " || Humidity1: " + h1);
    }
    if(t2 >= 0) {
      Serial.println(String("Temp2: ") + temp2 + " || Humidity2: " + h2);
    }
  
}

void measureBattery() {
  // Monitor the voltage divider on GPIO 32
  voltageDividerReading = 
      (analogRead(VOLTAGE_PIN) / 4095.0f) * 3.3f * voltageCalibrationFactor + voltageOffset;

  // Adjust the calibration factor to match the actual voltage
  voltageDividerReading *= (4.2 / 4.28);
  battery = voltageDividerReading; // IoT Cloud variable
  
  if(debug) {
    Serial.print("Voltage Divider Reading: ");
    Serial.println(voltageDividerReading);
  }
}


void updateScale() {
  WebSerial.println(String("LoadCell Reversed: ") + reversedloadcell);
  // Display weighing on OLED
  display.clearDisplay();
  display.setTextSize(2); // Set text size to 2 for larger text
  display.setTextColor(SSD1306_WHITE); // Set text color to white
  display.setCursor(0, 0); // Set cursor to top-left corner
  display.print("Weighing: ");
  display.display(); // Display the text
  static bool newDataReady = false;
  int sampleCount = 50;
  int total = 0 ;
if(debug){
Serial.println("Reading Scale");
WebSerial.println("Reading Scale");

esp_task_wdt_reset();
}
 
   for (int i = 0; i < sampleCount; i++) {
    while (!LoadCell.update()) {
      Serial.print("Reading Scale:  ");
      Serial.println(LoadCell.getData());
      display.clearDisplay();
      display.setTextSize(1); // Set text size to 2 for larger text
      display.setTextColor(SSD1306_WHITE); // Set text color to white
      display.setCursor(0, 0); // Set cursor to top-left corner
      display.print("Reading: ");
      display.setTextSize(2); // Set text size to 2 for larger text
      display.setCursor(0,15); // Set cursor to top-left corner
      display.print(LoadCell.getData());
      display.display(); // Display the text
    }
    total += LoadCell.getData();
   // Serial.println(String("Raw Data: ") + LoadCell.getData()*calibrationValue);
  }
  

      grams= total/sampleCount;

    
    if (grams >= -5 && grams <= 5) {
      grams = 0;
    }
    esp_task_wdt_reset();
      WebSerial.println(String("Last Weight: ") + last_weightstore);
      grams=grams+last_weightstore; //set offset from last weight

      if(grams < 0){
        WebSerial.println("Negative Weight Detected. Zeroing...");
        grams = 0;

      }
   
      //grams=LoadCell.getData();

 // mVA= movingAverage(grams);
  WebSerial.println(String("mVA: ") + mVA);
  WebSerial.println(String("Grams: ") + grams);
  // Convert grams to pounds (1 gram = 0.00220462 pounds)
  weightInPounds = grams * 0.00220462;
  WebSerial.println(String("Weight in Pounds: ") + weightInPounds);
  prefs.begin("beehive",false);
  prefs.putInt("Weight",grams);
  prefs.end();
    clearOLED();

}




void recalibrateScale(float knownWeight) {
  if(debug) {
    Serial.println("Recalibration started...");
    Serial.println("Scale tared.");
    Serial.println("Place the known weight on the scale.");
  }

  // 1. Tare the scale
  LoadCell.tareNoDelay();
  delay(5000); // Wait for user to place weight

  // 2. Read the raw value
  float rawValue = LoadCell.getData();
  if(debug) {
    Serial.print("Raw value read from HX711: ");
    Serial.println(rawValue);
  }

  // 3. Calculate new calibration factor
  float newCalFactor = rawValue / knownWeight;
  if(debug) {
    Serial.print("New Calibration Factor: ");
    Serial.println(newCalFactor);
  }

  // 4. Update the calibration factor
  LoadCell.setCalFactor(newCalFactor);

  // 5. Store the calibration factor
  prefs.begin("beehive-data");
  prefs.putFloat("calibrationFactor", newCalFactor);
  float check = prefs.getFloat("calibrationFactor", NAN);
  if(debug) {
    Serial.println(String("CHECK=  ") + check);
  }
  prefs.end();

  if(debug) {
    Serial.println("Calibration factor updated and saved.");
    Serial.println("Remove the weight.....");
  }
  delay(3000);
}


void reverseloadcell() {
  LoadCell.setReverseOutput();
  LoadCell.tareNoDelay();  

  }
