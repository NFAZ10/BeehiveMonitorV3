#include "webserialsetup.h"
#include "sensors.h"
#include "variables.h"
#include <HX711_ADC.h>
#include <Preferences.h>
#include "basicfunctions.h"
#include "ota.h"
#include <ESPDashPro.h>
#include <NetWizard.h>

AsyncWebServer server(80);  // Define WebServer instance
NetWizard NW(&server);
ESPDash dashboard(&server); 


Card card1(&dashboard, STATUS_CARD, "WiFi Status", DASH_STATUS_SUCCESS);
Card temperature(&dashboard, TEMPERATURE_CARD, "Ext Temperature", "°F");
Card humidity(&dashboard, HUMIDITY_CARD, "Ext Humidity", "%");
Card weightcard(&dashboard, GENERIC_CARD, "Weight", "g");




Card resetNW(&dashboard, PUSH_BUTTON_CARD, "Reset Wifi Setup");
Card tarecard(&dashboard, PUSH_BUTTON_CARD, "Tare Scale");
Card reverseloadcellcard(&dashboard, BUTTON_CARD, "Reverse Load Cell");
Card linkcard(&dashboard, LINK_CARD, "Serial Monitor");

Tab tab1(&dashboard, "Settings");

void setcardtabs(){
resetNW.setTab(&tab1);
tarecard.setTab(&tab1);
reverseloadcellcard.setTab(&tab1);
linkcard.setTab(&tab1);
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

// Setup configuration parameters

extern Preferences prefs;

void wifiSetup(){

  card1.attachCallback([&](){
    Serial.println("[Card1] Push Button Triggered");
    NW.erase();
  });

  NW.autoConnect("NetWizard Demo", "");  // <-- Add this line
  NW.setStrategy(NetWizardStrategy::NON_BLOCKING);
  NW.setPortalTimeout(1000);
  linkcard.update("/webserial");
 

    if (NW.getConnectionStatus() == NetWizardConnectionStatus::CONNECTED) {
      // Print network details
      Serial.print("Connected to ");
      Serial.println(NW.getSSID());
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());;
      card1.update("Connected", DASH_STATUS_SUCCESS);
      NW.stopPortal();
      delay(1000);
      webserial();
    } else {
      Serial.println("Not connected to any WiFi");
      card1.update("Not Connected", DASH_STATUS_WARNING);
    }


}

void NWLoop(){
  NW.loop();
}


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
    } else if(msg == "REVERSE") {
      WebSerial.println("Reversing Load Cell");
     
      reversedloadcell = !reversedloadcell;
      WebSerial.println(reversedloadcell);
      prefs.begin("beehive", false);
      prefs.putBool("reversedloadcell",reversedloadcell);
      prefs.end();
      WebSerial.println("Reversed Load Cell");
     
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
      prefs.putBool("newSetups",false);
      prefs.end();
     

    }else if(msg.startsWith("NAMESET[")){
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
      
      } else
  
    {
      /* code */
    }
    
  
    WebSerial.println(msg);  // Send the reconstructed string to WebSerial
  
  }


  void webserial() {
    Serial.println("Initializing WebSerial && DashPro...");
    WebSerial.begin(&server); // Start WebSerial
    WebSerial.onMessage(recvMsg);      
    server.begin();     // Start WebServer
    WebSerial.println("WebSerial initialized!");

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
      prefs.begin("beehive", false);
      prefs.putBool("reversedloadcell",reversedloadcell);
      prefs.end();
      WebSerial.println("Reversed Load Cell");
     
      reverseloadcellcard.update(1);
      dashboard.sendUpdates();
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