#include <Arduino.h>
#include <PubSubClient.h>
#include "variables.h"
#include <WiFi.h>
#include <WebSerial.h>
#include "basicfunctions.h"
#include <Preferences.h>

WiFiClient espClient;
PubSubClient mqttClient(espClient); 
String macAddress;
extern Preferences prefs;
const char* mqttServerb = "mqtt.beehivemonitor.com";
const int mqttPortb = 4116;

extern String Name;

#include <WiFi.h>  // Ensure WiFi library is included
#include <Arduino.h>

String getMacAddress() {
    if (Name != NULL && Name.length() > 0) {
        return Name;
    }else{
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(macStr);
}}

void connectToMQTT() {
    String macAddress = getMacAddress();
    while (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (mqttClient.connect("BeehiveMonitor")) {
            Serial.println("connected");

            } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
}

void initMQTT() {
    mqttClient.setServer(mqttServerb, mqttPortb);
    mqttClient.setCallback(mqttCallback);
    WebSerial.println("MQTT Client Initialized.");

    // Retrieve stored values from NVS
    prefs.begin("beehive", true); // Open in read-only mode
    newSetup = prefs.getBool("newSetup", false); // Default to false if not set
    disablesleep = prefs.getBool("disableSleep", false); // Default to false if not set
    prefs.end();

    // Print retrieved values
    Serial.print("Retrieved newSetup: ");
    Serial.println(newSetup);
    Serial.print("Retrieved disableSleep: ");
    Serial.println(disablesleep);
}
