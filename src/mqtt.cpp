#include <Arduino.h>
#include <PubSubClient.h>
#include "variables.h"
#include <WiFi.h>
#include <WebSerial.h>

WiFiClient espClient;
PubSubClient mqttClient(espClient); 
String macAddress;

const char* mqttServerb = "mqtt.beehivemonitor.com";
const int mqttPortb = 4116;


void initMQTT() {
  // Configure the MQTT broker
  mqttClient.setServer(mqttServerb, mqttPortb);
  WebSerial.println("MQTT Client Initialized.");
  

}

#include <WiFi.h>  // Ensure WiFi library is included
#include <Arduino.h>

void connectToMQTT() {
    int retryCount = 0;
    const int maxRetries = 10;

    while (!mqttClient.connected()) {
        Serial.print("Connecting to MQTT...");
        
       
        if (mqttClient.connect("clientId")) {
            Serial.println("connected");

            // Subscribe to the tare topic
            String topicBase = "beehive/data/" + String(WiFi.macAddress()) + "/cmd";
            mqttClient.subscribe(topicBase.c_str());
            Serial.println("Subscribed to topic: " + topicBase);
            return;
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");

            retryCount++;
            if (retryCount >= maxRetries && WiFi.status() == WL_CONNECTED) {
                Serial.println("Max retries reached, rebooting...");
                delay(1000);
                ESP.restart();
            }
            delay(5000);
        }
    }
}