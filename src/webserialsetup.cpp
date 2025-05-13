#include "webserialsetup.h"
#include "sensors.h"
#include "variables.h"
#include <HX711_ADC.h>
#include <Preferences.h>
#include "basicfunctions.h"
#include "ota.h"
#include <ESPDashPro.h>
#include <NetWizard.h>
#include "NAU7204.h"
#include <Preferences.h>


AsyncWebServer server(80);  // Define WebServer instance

extern Preferences prefs;
ESPDash dashboard(&server,"/",false); 
NetWizard NW(&server);

Card card1(&dashboard, STATUS_CARD, "WiFi Status", DASH_STATUS_SUCCESS);
Card temperature(&dashboard, TEMPERATURE_CARD, "Ext Temperature", "°F");
Card humidity(&dashboard, HUMIDITY_CARD, "Ext Humidity", "%");
Card weightcard(&dashboard, GENERIC_CARD, "Weight", "g");
Card calibration(&dashboard, GENERIC_CARD, "CalValue", "");
Card offset(&dashboard, GENERIC_CARD, "ZeroOffset", "");



Card cellconfig(&dashboard, DROPDOWN_CARD, "Loadcell Config", "Front and Back,Front Only,Back Only");



Card resetNW(&dashboard, PUSH_BUTTON_CARD, "Reset Wifi Setup");
Card tarecard(&dashboard, PUSH_BUTTON_CARD, "Tare Scale");
Card reverseloadcellcard(&dashboard, BUTTON_CARD, "Reverse Load Cell");
Card linkcard(&dashboard, LINK_CARD, "Serial Monitor");


Tab tab2(&dashboard, "Configuration");
Tab tab1(&dashboard, "Settings");

void setcardtabs(){
resetNW.setTab(&tab1);
tarecard.setTab(&tab1);
reverseloadcellcard.setTab(&tab1);
linkcard.setTab(&tab1);

cellconfig.setTab(&tab2);



}



void updateEXTTemp(float temp) {
  temperature.update(temp);
}
void updateEXTHum(float hum) {
  humidity.update(hum);
}

void updateweightcard(float dgrams) {
  weightcard.update(dgrams);
}

void updatecalcard(float dgrams) {
  calibration.update(dgrams);
}
void updatezerocard(float dgrams) {
  offset.update(dgrams);
}

void attachcallbacks(){

  cellconfig.attachCallback([&](const char* value){
    Serial.println("Dropdown Callback Triggered: "+String(value));
    /* Dropdown card updater - you need to update the button with latest value upon firing of callback */
    if (strcmp(value, "Front and Back") == 2) {
      prefs.begin("beehive-data", false);
      prefs.putInt("loadcellconfig", 2);
      prefs.end();
    } else if (strcmp(value, "Front Only") == 0) {
      prefs.begin("beehive-data", false);
      prefs.putInt("loadcellconfig", 0);
      prefs.end();
    } else if (strcmp(value, "Back Only") == 0) {
      prefs.begin("beehive-data", false);
      prefs.putInt("loadcellconfig", 0);
      prefs.end();
    } else {
      prefs.begin("beehive-data", false);
      prefs.putInt("loadcellconfig", 0);
      prefs.end();
      
    }
    cellconfig.update(value);
    /* Send update to dashboard */

    dashboard.sendUpdates();


  });

}

// Setup configuration parameters

extern Preferences prefs;

void wifiSetup(){

  NW.setStrategy(NetWizardStrategy::BLOCKING);
  NW.autoConnect("BeehiveMonitorAP", "");  // <-- Add this line
  NW.setPortalTimeout(30000);
  
}

void NWLoop(){
  setcardtabs();
  NW.loop();
}

bool awaitingCalibrationInput = false;


void recvMsg(uint8_t *data, size_t len) {
  WebSerial.println("Received Data...");

  String msg = "";  // Initialize an empty string
  for (size_t i = 0; i < len; i++) {
    msg += (char)data[i];
  }

  msg.trim();  // Remove whitespace and \r\n
  WebSerial.println("Received Data: " + msg);  // Print the received message

  if (awaitingCalibrationInput) {
    float cal = msg.toFloat();
    if (cal != 0.0 || msg == "0" || msg == "0.0") {
      calibrationValue = cal;
      prefs.begin("beehive-data", false);
      prefs.putFloat("calFactor", calibrationValue);
      prefs.end();
      WebSerial.println("Calibration Value Set: " + String(calibrationValue));
      awaitingCalibrationInput = false;
      delay(1000);
      WebSerial.println("Rebooting...");
      delay(1000);
      ESP.restart();
    } else {
      WebSerial.println("Invalid calibration input. Try again:");
    }
    return;
  }

  // === Existing commands ===
  if (msg == "tare") {
    tareRequested = true;
    WebSerial.println("Tare requested");
  }

  else if (msg == "CAL") {
    WebSerial.println("Enter calibration value:");
    //waitForCalibrationInput();  // Custom blocking function
  }
  

  else if (msg == "lowpower") {
    lowPowerMode = !lowPowerMode;
    WebSerial.println("Low Power Mode Toggled");
  }

  else if (msg == "reboot") {
    WebSerial.println("Rebooting...");
    ESP.restart();
  }

  else if (msg.startsWith("SET[")) {
    int startIndex = msg.indexOf('[') + 1;
    int endIndex = msg.indexOf(']');
    if (startIndex > 0 && endIndex > startIndex) {
      String weightStr = msg.substring(startIndex, endIndex);
      last_weightstore = weightStr.toInt();
    }
    WebSerial.println("Weight Set");
  }

  else if (msg.startsWith("CALSET[")) {
    int startIndex = msg.indexOf('[') + 1;
    int endIndex = msg.indexOf(']');
    if (startIndex > 0 && endIndex > startIndex) {
      String calibrationStr = msg.substring(startIndex, endIndex);
      calibrationValue = calibrationStr.toFloat();
      WebSerial.println("Calibration Value: " + String(calibrationValue));
      myScale.setCalibrationFactor(calibrationValue);

      prefs.begin("beehive",false);
      prefs.putFloat("calFactor", calibrationValue);
      float testvalue = prefs.getFloat("calFactor", 0.0);
      WebSerial.println("Test Value: " + String(testvalue));

      prefs.end();
      delay(1000);
      WebSerial.println("Calibration Set");
      WebSerial.println(calibrationValue);
      WebSerial.println("Rebooting...");
      delay(1000);
     // ESP.restart();
    }
  }  
  
  
  else if (msg.startsWith("OFFSET[")) {
    int startIndex = msg.indexOf('[') + 1;
    int endIndex = msg.indexOf(']');
    if (startIndex > 0 && endIndex > startIndex) {
      String zeroOffsetstr = msg.substring(startIndex, endIndex);
     int zeroOffset = zeroOffsetstr.toInt();
      WebSerial.println("Offset Value: " + String(zeroOffset));
      myScale.setZeroOffset(zeroOffset);

      prefs.begin("beehive",false);
      prefs.putInt("zeroOffset", zeroOffset);
      float testvalueoff = prefs.getInt("zeroOffset", 0);
      WebSerial.println("Test Value: " + String(testvalueoff));

      prefs.end();
      delay(1000);
      WebSerial.println("Offset Set");
      WebSerial.println(testvalueoff);
      WebSerial.println("Rebooting...");
      delay(1000);
     // ESP.restart();
    }
  }else if(msg == "loadcellconfig[0]"){
    prefs.begin("beehive",false);
    prefs.putInt("loadcellconfig", 0);
    WebSerial.println("Loadcell Config: 0");
    prefs.end();
  }else if(msg == "loadcellconfig[1]"){
    prefs.begin("beehive",false);
    prefs.putInt("loadcellconfig", 1);
    WebSerial.println("Loadcell Config: 1");
    prefs.end();
  }

  else if (msg == "debug") {
    debug = !debug;
    WebSerial.println("Debug Mode Toggled");
  }

  else if (msg == "factoryreset") {
    Serial.println("Factory Reset Button Pressed");
    newSetup = true; // Set the flag when the interrupt is triggered
    clearPreferences();
    ESP.restart();
  }

  else if (msg == "printprefs") {
    printPreferences();
  }

  else if (msg == "clear") {
    clearPreferences();
    WebSerial.println("Preferences Cleared");
  }

  else if (msg == "debug") {
    debug = !debug;
  }

  else if (msg == "sleep") {
    disablesleep = false;
    WebSerial.println("Sleep Enabled");
    delay(1000);
    ESP.restart();
  }

  else if (msg == "wake") {
    disablesleep = true;
    WebSerial.println("Sleep Disabled");
  }

  else if (msg == "otaforce") {
    performOTA();
  }

  else if (msg == "REVERSE") {
    WebSerial.println("Reversing Load Cell");
    reversedloadcell = 1;
    WebSerial.println(reversedloadcell);
    prefs.begin("beehive", false);
    prefs.putInt("reversed", 1);
    prefs.end();
    WebSerial.println("Reversed Load Cell");
    WebSerial.println("Rebooting...");

    if (reversedloadcell == 1) {
      WebSerial.println("******Reversed Load Cell*******");
      reverseloadcell();
    }
    delay(1000);
  }

  else if (msg == "nauCal") {
    nauCalRequested = true;
  }

  else if (msg == "pref") {
    // loadPreferences();
  }

  else if (msg == "read") {
    // readDHTSensors();
  }

  else if (msg == "activate") {
    WebSerial.println("Activating install from Factory Defaults");
    newSetup = false;
    prefs.begin("beehive", false);
    prefs.putBool("newSetups", false);
    prefs.end();
  }

  else if (msg.startsWith("NAMESET[")) {
    prefs.begin("beehive", false);
    int startIndex = msg.indexOf('[') + 1;
    int endIndex = msg.indexOf(']');
    if (startIndex > 0 && endIndex > startIndex) {
      String nameStr = msg.substring(startIndex, endIndex);
      prefs.putString("name", nameStr);
    }
    prefs.end();
    WebSerial.println("Name Set");
    WebSerial.println(Name);
  }

  WebSerial.println(msg);  // Echo message
}



  void webserial() {
    Serial.println("Initializing WebSerial && DashPro...");
    WebSerial.begin(&server); // Start WebSerial
    WebSerial.onMessage(recvMsg);      
    server.begin();     // Start WebServer
    WebSerial.println("WebSerial initialized!");
    linkcard.update("/webserial");

    resetNW.attachCallback([&](){
      Serial.println("[Card1] Push Button Triggered");
      NW.erase();

      delay(1000);
      NW.startPortal();

    });
    tarecard.attachCallback([&](){
      Serial.println("Tare Button Triggered");
      tareRequested = true;
    });

    reverseloadcellcard.attachCallback([&](int value){
      Serial.println("[Card1] Button Callback Triggered: "+String((value == 1)?"true":"false"));
      reverseloadcellcard.update(value);
      reversedloadcell = !reversedloadcell;
      WebSerial.println(String("Loadcell ")+reversedloadcell);
      prefs.begin("beehive", false);
      prefs.putBool("reversed",reversedloadcell);
      prefs.end();
      WebSerial.println("Reversed Load Cell");
     
      reverseloadcellcard.update(1);
      dashboard.sendUpdates();
      WebSerial.println("Rebooting...");
      delay(1000);
     // ESP.restart();
    });
    

}


void reconnectWifi(){
  NW.connect();
}
void disconnectWifi(){
  NW.disconnect();
}

void dashLoop(){
  setcardtabs();
  dashboard.sendUpdates();
}