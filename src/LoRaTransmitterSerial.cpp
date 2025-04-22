#include "LoRaTransmitterSerial.h"

#define LORA_SERIAL Serial2
#define LORA_TX 17  // ESP32 TX -> LoRa RX
#define LORA_RX 16  // ESP32 RX <- LoRa TX
#define LORA_BAUD 9600

void setupLoRaTransmitter() {
    LORA_SERIAL.begin(LORA_BAUD, SERIAL_8N1, LORA_RX, LORA_TX);
    Serial.println("✅ LoRa UART transmitter ready.");
}

void sendHiveData(float weight, float temp1, float temp2, float humidity, float sound, int beeCount) {
    String packet = String(weight) + "," + String(temp1) + "," + String(temp2) + "," +
                    String(humidity) + "," + String(sound) + "," + String(beeCount) + "\n";
    LORA_SERIAL.print(packet);
    Serial.print("📡 Sent over LoRa UART: ");
    Serial.print(packet);
}
