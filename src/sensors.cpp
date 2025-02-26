#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include "HX711_ADC.h"
#include "DHT.h"
#include "ArduinoJson.h"
#include "Adafruit_NeoPixel.h"
#include <Preferences.h>
#include <ArduinoJson.h>
#include "variables.h"
#include "webserialsetup.h"




Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


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

void tareScale();
float movingAverage(int newValue);
void processCommand(String command);

void initDHTSensors() {
    dht1.begin();
    dht2.begin();
  }

  void initScale() {

    if(debug) Serial.println("Initializing HX711...");
    LoadCell.begin();
    
    LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  
  
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




  void tareScale() {
    // Start the tare process
  
    LoadCell.update();
  
     LoadCell.refreshDataSet();
     LoadCell.resetSamplesIndex();
     //LoadCell.setTareOffset(0);
  
  
    LoadCell.tareNoDelay();
    if(debug) {
      Serial.println("Tare started...");
    }
  
    // Reset variables after taring
    grams  = 0;
    mVA    = 0;
    weight = 0;
    prefs.begin("beehive-data",false);
    prefs.putInt("lastWeight", grams);
    prefs.putInt("Weight", grams);
    prefs.putFloat("mVA", mVA);
    prefs.end();
  
  
  
    if(debug) {
      Serial.println("Tare completed:");
      Serial.println("Raw Grams: 0");
      Serial.println("Moving Avg Grams: 0");
      Serial.println("Weight (oz): 0");
    }
  
  
  
    if(debug) {
      Serial.println("Rebooting after tare...");
    }
    delay(500); // Allow Serial message to complete
    weightset=false;
    //ESP.restart(); // Reboot the ESP
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
  static bool newDataReady = false;
  int sampleCount = 10;
  int total = 0 ;
if(debug){
Serial.println("Reading Scale");
      strip.setPixelColor(0,255,255,15); //  Set pixel's color (in RAM)
      strip.show();


}
 
   for (int i = 0; i < sampleCount; i++) {
    while (!LoadCell.update()) {
      //Serial.print("Reading Scale:  ");
      //Serial.println(LoadCell.getData());
    }
    total += LoadCell.getData();
   // Serial.println(String("Raw Data: ") + LoadCell.getData()*calibrationValue);
  }
  
       //Serial.print(String("Total: ") + total); Serial.println(String(" || SampleCount: ") + sampleCount);
      
      grams= total/sampleCount;
      Serial.println(String("Last Weight: ") + last_weightstore);
      grams=grams+last_weightstore; //set offset from last weight

      if(grams < 0){
        Serial.println("Negative Weight Detected. Taring...");
        grams = 0;
       
        tareScale();
      }
   
      //grams=LoadCell.getData();

  mVA= movingAverage(grams);
  Serial.println(String("Grams: ") + grams);
  // Convert grams to pounds (1 gram = 0.00220462 pounds)
  weightInPounds = grams * 0.00220462;
  Serial.println(String("Weight in Pounds: ") + weightInPounds);
    
      strip.setPixelColor(0,0,0,0); //  Set pixel's color (in RAM)
      strip.show();
}

float movingAverage(int newValue) {
  static float readings[NUM_SAMPLES] = {0};
  static int   index = 0;
  static float sum   = 0;


  // Subtract the oldest reading from sum
  sum -= readings[index];

  // Store the new reading
  readings[index] = newValue;
  sum += newValue;

  // Advance the index, wrapping around
  index = (index + 1) % NUM_SAMPLES;

  // Return the average
  return sum / NUM_SAMPLES;
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

void handleWebSerialInput(uint8_t *data, size_t len);  // Function to handle WebSerial input



void handleWebSerialInput(uint8_t *data, size_t len) {
    String command = "";
    for (size_t i = 0; i < len; i++) {
        command += (char)data[i];
    }
    command.trim();  // Remove whitespace

    WebSerial.println("Received: " + command);  // Echo back to WebSerial
    processCommand(command);
}

// Handles both Serial and WebSerial commands
void handleSerialCommands() {
    if (debug && Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        processCommand(command);
    }
}

bool waitingForCalibration = false;  // Flag to track calibration input
float knownWeight = 0;  // Known weight for calibration
void processCommand(String command) {
    if (waitingForCalibration) {
        // User has entered the weight after "CAL" prompt
            knownWeight = command.toFloat();
        if (knownWeight > 0) {
            recalibrateScale(knownWeight);
            Serial.println("Calibration complete with known weight: " + String(knownWeight));
            WebSerial.println("Calibration complete with known weight: " + String(knownWeight));
            waitingForCalibration = false;  // Reset flag
        } else {
            Serial.println("Invalid weight. Please enter a valid number.");
            WebSerial.println("Invalid weight. Please enter a valid number.");
        }
        return;  // Exit after handling calibration
    }

    if (command.startsWith("CAL")) {
        Serial.println("Enter known weight for calibration:");
        WebSerial.println("Enter known weight for calibration:");
        waitingForCalibration = true;  // Set flag to wait for input
    }
    else if (command.startsWith("SET")) {
        calibrationValue = command.substring(3).toFloat();
        LoadCell.setCalFactor(calibrationValue);
        Serial.println("Cal Set");
        WebSerial.println("Cal Set");
    }else if (command.startsWith("TARE")) {
        tareScale();
    }
    else if (command.startsWith("WEIGHT")) {
        
        last_weightstore = command.substring(6).toInt();
        Serial.println("Weight Set");
        WebSerial.println("Weight Set");
    }else {
        Serial.println("Unknown command");
        WebSerial.println("Unknown command");
    }






}