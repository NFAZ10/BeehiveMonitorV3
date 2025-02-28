#ifndef MQTT_H
#define MQTT_H

#include <PubSubClient.h>

extern PubSubClient mqttClient;

void initAWS();
void connectToAWS();
void connectToMQTT();
void initMQTT();
#endif
