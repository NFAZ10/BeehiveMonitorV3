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
            // Subscribe to topics with MAC address
            mqttClient.subscribe(("beehive/" + macAddress + "/command/tare").c_str());
            mqttClient.subscribe(("beehive/" + macAddress + "/command/reset").c_str());
            mqttClient.subscribe(("beehive/" + macAddress + "/command/sleep").c_str());
            mqttClient.subscribe(("beehive/" + macAddress + "/command/wakeup").c_str());
            mqttClient.subscribe(("beehive/" + macAddress + "/command/disableSleep").c_str());
            mqttClient.subscribe(("beehive/" + macAddress + "/command/newSetup").c_str());
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
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    Serial.println(message);

    String macAddress = getMacAddress();
    String baseTopic = "beehive/" + macAddress + "/command/";

    // Act on the message based on the topic
    if (String(topic) == baseTopic + "tare") {
        tareRequested = true;
    } else if (String(topic) == baseTopic + "reset") {
        ESP.restart();
    } else if (String(topic) == baseTopic + "wakeup") {
        disablesleep = true;
    } else if (String(topic) == baseTopic + "disableSleep") {
        disablesleep = message.toInt();
    } else if (String(topic) == baseTopic + "newSetup") {
        newSetup = message.toInt();
        prefs.begin("beehive", false);
        prefs.putBool("newSetup", newSetup);
        prefs.end();
    }
}


void initMQTT() {
    mqttClient.setServer(mqttServerb, mqttPortb);
    mqttClient.setCallback(mqttCallback);
    WebSerial.println("MQTT Client Initialized.");
}
