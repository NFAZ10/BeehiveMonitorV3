#ifndef LORA_TRANSMITTER_SERIAL_H
#define LORA_TRANSMITTER_SERIAL_H

#include <Arduino.h>

void setupLoRaTransmitter();
void sendHiveData(float weight, float temp1, float temp2, float humidity, float sound, int beeCount);

#endif
