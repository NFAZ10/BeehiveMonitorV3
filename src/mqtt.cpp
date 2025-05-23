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
float mva24;

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
        if (mqttClient.connect("BeehiveMonitor")) {
            Serial.println("connected");
            String topicPrefix = "beehive/data/" + macAddress;
            // Subscribe to backend topics
            mqttClient.subscribe((topicPrefix + "/backend/battery").c_str());
            WebSerial.println("Subscribed to backend battery topic.");
            mqttClient.subscribe((topicPrefix + "/backend/version").c_str());
            WebSerial.println("Subscribed to backend version topic.");
            mqttClient.subscribe((topicPrefix + "/backend/IP").c_str());
            WebSerial.println("Subscribed to backend IP topic.");
            mqttClient.subscribe((topicPrefix + "/backend/charging").c_str());
            WebSerial.println("Subscribed to backend charging topic.");
            mqttClient.subscribe((topicPrefix + "/backend/NAU7802").c_str());
            WebSerial.println("Subscribed to backend NAU7802 topic.");
            mqttClient.subscribe((topicPrefix + "/backend/CalValue").c_str());
            WebSerial.println("Subscribed to backend CalValue topic.");
            mqttClient.subscribe((topicPrefix + "/backend/loadcellconfig").c_str());
            WebSerial.println("Subscribed to backend loadcellconfig topic.");
            mqttClient.subscribe((topicPrefix + "/backend/mva24h").c_str());
            WebSerial.println("Subscribed to weight topic.");


            WebSerial.println("Subscribed to backend topics.");
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            static int retryCount = 0;
            delay(100);
            retryCount++;
            if (retryCount >= 5) {
                Serial.println("Failed to connect to MQTT after 5 attempts. Moving on...");
                break;
            }
        }
    }
}


void mqttCallback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0'; // Null-terminate the payload
    String incomingPayload = String((char*)payload);
    String incomingTopic = String(topic);

    WebSerial.println("Message arrived [" + incomingTopic + "]: " + incomingPayload);

    if (incomingTopic.endsWith("/backend/CalValue")) {
        prefs.begin("beehive", false);
        prefs.putFloat("calFactor",incomingPayload.toFloat());
        prefs.end();
        WebSerial.println("Calibration Value Set: " + String(incomingPayload.toFloat()));
    } else  if (incomingTopic.endsWith("/backend/ZeroOffset")) {
        prefs.begin("beehive", false);
        prefs.putFloat("zeroOffset",incomingPayload.toFloat());
        prefs.end();
        WebSerial.println("Zero Offset Set: " + String(incomingPayload.toFloat()));
    }else if (incomingTopic.endsWith("/backend/loadcellconfig")) {
        prefs.begin("beehive", false);
        prefs.putInt("loadcellconfig",incomingPayload.toInt());
        prefs.end();
        WebSerial.println("Load Cell Config Set: " + String(incomingPayload.toInt()));
    }if (incomingTopic.endsWith("/backend/mva24h")) {
        float mva24 = incomingPayload.toFloat();  // ← Set grams here
        WebSerial.println("MVA4 (from backend/mva24h) set to: " + String(mva24));
    }
    
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
