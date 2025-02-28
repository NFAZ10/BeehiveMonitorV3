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

void connectToMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "ESP32Client-" + String(WiFi.macAddress());
    if (mqttClient.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println("connected");
      // Subscribe to the tare topic
      String topicBase = "beehive/data/" + String(WiFi.macAddress()) + "/cmd";
      mqttClient.subscribe(topicBase.c_str());
      Serial.println("Subscribed to topic: " + topicBase);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
