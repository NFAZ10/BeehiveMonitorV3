#include "webserialsetup.h"
#include "sensors.h"
#include "variables.h"
#include <HX711_ADC.h>
#include <Preferences.h>
#include "basicfunctions.h"
#include "ota.h"


AsyncWebServer server(80);  // Define WebServer instance

extern Preferences prefs;

void recvMsg(uint8_t *data, size_t len) {
    WebSerial.println("Received Data...");
  
    String msg = "";  // Initialize an empty string
    for (size_t i = 0; i < len; i++) {
      msg += (char)data[i];  // Append each character to the string
    }
  
    if (msg == "tare") {
      tareRequested = true;
      WebSerial.println("Tare requested");
    } else if (msg == "lowpower") {
      lowPowerMode = !lowPowerMode;
      WebSerial.println("Low Power Mode Toggled");
    } else if (msg == "reboot") {
      WebSerial.println("Rebooting...");
      ESP.restart();
    } else if (msg.startsWith("SET[")) {
      int startIndex = msg.indexOf('[') + 1;
      int endIndex = msg.indexOf(']');
      if (startIndex > 0 && endIndex > startIndex) {
      String weightStr = msg.substring(startIndex, endIndex);
      last_weightstore = weightStr.toInt();
      }
      WebSerial.println("Weight Set");
    }  else if(msg.startsWith("CALSET[")){
      int startIndex = msg.indexOf('[') + 1;
      int endIndex = msg.indexOf(']');
      if (startIndex > 0 && endIndex > startIndex) {
      String calibrationStr = msg.substring(startIndex, endIndex);
      calibrationValue = calibrationStr.toFloat();
      prefs.begin("beehive-data");
      prefs.putFloat("calibrationFactor", calibrationValue);
      prefs.end();
      ESP.restart();
      }
      WebSerial.println("Cal Set");
    } else if(msg == "clear") {
     clearPreferences();
      WebSerial.println("Preferences Cleared");
    } else if(msg == "debug") {
      debug = !debug;
      WebSerial.println("Debug Set");
    } else if(msg == "sleep") {
     disablesleep = false;
    } else if(msg == "wake") {
     disablesleep = true;
    } else if(msg == "otaforce") {
        performOTA();
     // connectToWiFi();
    } else if(msg == "ap") {
     // createAccessPointIfNeeded();
    } else if(msg == "off") {
   //   turnOffWiFi();
    } else if(msg == "pref") {
   //   loadPreferences();
    } else if(msg == "read") {
    //  readDHTSensors();
    } else if(msg == "activate") {
      WebSerial.println("Activating install from Factory Defaults");
      newSetup = false;
      prefs.begin("beehive", false);
      prefs.putBool("newSetup",false);
      prefs.end();
      delay(1000);
      ESP.restart();

    }else if(msg.startsWith("NAMESET[")){
      prefs.begin("beehive", false);
      int startIndex = msg.indexOf('[') + 1;
      int endIndex = msg.indexOf(']');
      if (startIndex > 0 && endIndex > startIndex) {
        String nameStr = msg.substring(startIndex, endIndex);
        prefs.putString("name", nameStr);
      }
      prefs.end();
      
      } else
  
    {
      /* code */
    }
    
  
    WebSerial.println(msg);  // Send the reconstructed string to WebSerial
  
  }

  void webserial() {
    WebSerial.begin(&server); // Start WebSerial
    WebSerial.onMessage(recvMsg);
    server.begin();           // Start WebServer
    WebSerial.println("WebSerial initialized!");
}
