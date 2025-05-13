#ifndef MQTT_H
#define MQTT_H

#include <PubSubClient.h>

extern PubSubClient mqttClient;
extern float mva24;

void initAWS();
void connectToAWS();
void connectToMQTT();
void initMQTT();
#endif
