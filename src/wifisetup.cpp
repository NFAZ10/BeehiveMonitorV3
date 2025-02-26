#include <Arduino.h>
#include <WiFiManager.h>

WiFiManager wifiManager;



void wmsetup() {

  Serial.println("Starting WiFi Manager");
  wifiManager.autoConnect("BeehiveMonitorWiFi");
  Serial.println("Connected to WiFi");

  wifiManager.setConfigPortalBlocking(false);
  wifiManager.setConfigPortalTimeout(60);


}