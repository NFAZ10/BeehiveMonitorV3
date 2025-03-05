#include <Arduino.h>
#include <PubSubClient.h>
#include "variables.h"
#include <WiFi.h>
#include <WebSerial.h>
#include <Adafruit_NeoPixel.h>

WiFiClient espClient;
PubSubClient mqttClient(espClient); 
String macAddress;

extern Adafruit_NeoPixel strip;

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
            strip.setPixelColor(0,0,255,0); //  Set pixel's color (in RAM)
            strip.show();

            // Subscribe to the tare topic
            String topicBase = "beehive/data/" + String(WiFi.macAddress()) + "/cmd";
            mqttClient.subscribe(topicBase.c_str());
            Serial.println("Subscribed to topic: " + topicBase);
            return;
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            WebSerial.println("MQTT Connection Failed. Retrying...");
            strip.setPixelColor(0,255,0,0); //  Set pixel's color (in RAM)
            strip.show();

            retryCount++;
            if (retryCount >= maxRetries && WiFi.status() == WL_CONNECTED) {
                Serial.println("Max retries reached, rebooting...");
                WebSerial.println("Max retries reached, rebooting...");
                delay(1000);
                ESP.restart();
            }
            delay(5000);
        }
    }
}