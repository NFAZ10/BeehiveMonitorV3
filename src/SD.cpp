#include <SPI.h>
#include <SdFat.h>
#include <Arduino.h>
// Define custom SPI pins
#define SD_MISO  9  // Change if needed
#define SD_MOSI  5  // Change if needed
#define SD_SCK   4  // Change if needed
#define SD_CS     2  // Custom Chip Select pin

SdFat SD;

void SDsetup() {
    
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  
    if (!SD.begin(SD_CS)) {
        Serial.println("❌ SD Card initialization failed!");
        return;
    }
    Serial.println("✅ SD Card initialized successfully.");
}
