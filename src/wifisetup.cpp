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

void resetWiFiManager() {
  Serial.println("Resetting WiFi Manager settings...");
  wifiManager.resetSettings();
  Serial.println("WiFi Manager settings have been reset.");
}