#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "mqtt.h"
#include <WiFi.h>
#include "variables.h"

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // I2C Address

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define WIFI_SYMBOL_WIDTH 16
#define WIFI_SYMBOL_HEIGHT 16
static const unsigned char PROGMEM wifiSymbol[] = {
  0x00, 0x00, 0x0F, 0xF0, 0x30, 0x0C, 0x40, 0x02, 0x8F, 0xF1, 0x10, 0x08, 0x20, 0x04, 0x47, 0xE2,
  0x80, 0x01, 0x8F, 0xF1, 0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00
};

#define MQTT_SYMBOL_WIDTH 16
#define MQTT_SYMBOL_HEIGHT 16
static const unsigned char PROGMEM mqttSymbol[] = {
  0x00, 0x00, 0x3F, 0xFC, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04,
  0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x3F, 0xFC, 0x00, 0x00, 0x00, 0x00
};

// Beehive data variables
extern float grams;  // Weight in grams
extern float t1;     // Temperature
extern float h1;     // Humidity
extern float voltageDividerReading;  // Battery voltage
extern bool debug;
extern const char* currentVersion;


void setupOLED() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);  // Loop forever if display fails
    }

    display.clearDisplay();
    display.setTextSize(1);      // Small font
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Beehive Monitor v3.0");
    display.setCursor(0, 10);
    display.print("Version: ");
    display.print(currentVersion);
    display.display();
    delay(2000);
}

// Function to update OLED with beehive data
void updateOLED() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Display weight
    display.setCursor(0, 0);
    display.print("Weight: ");
    display.print(grams);
    display.println(" g");

    // Display temperature and humidity
    display.setCursor(0, 10);
    display.print("Temp: ");
    display.print(t1);
    display.print(" C  ");


    // Display battery level
    display.setCursor(0, 20);
    display.print("Battery: ");
    display.print(voltageDividerReading);
    display.print(" V");



    display.display();  // Show the updated display
}

void clearOLED() {
    display.clearDisplay();
    display.display();
    delay(2000);
}

void updateOLEDWithNetworkStatus() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Display WiFi status
    display.setCursor(0, 0);
    if (WiFi.status() == WL_CONNECTED) {
        display.drawBitmap(0, 0, wifiSymbol, WIFI_SYMBOL_WIDTH, WIFI_SYMBOL_HEIGHT, SSD1306_WHITE);
        display.setCursor(18, 0);
        display.print("WiFi: Connected");
    } else {
        display.print("WiFi: Disconnected");
    }

    // Display MQTT status
    display.setCursor(0, 20);
    if (mqttClient.connected()) {
        display.drawBitmap(0, 20, mqttSymbol, MQTT_SYMBOL_WIDTH, MQTT_SYMBOL_HEIGHT, SSD1306_WHITE);
        display.setCursor(18, 20);
        display.print("MQTT: Connected");
    } else {
        display.print("MQTT: Disconnected");
    }

    // Display IP address
    display.setCursor(0, 40);
    if (WiFi.status() == WL_CONNECTED) {
        display.print("IP: ");
        display.print(WiFi.localIP());
    } else {
        display.print("IP: N/A");
    }

    display.display();  // Show the updated display
}

void powerOffOLED() {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
}

void powerOnOLED() {
    display.ssd1306_command(SSD1306_DISPLAYON);
}

void printToOLED(String message) {
    display.clearDisplay();
    int16_t x = 0;
    int16_t y = 0;
    int16_t textWidth = message.length() * 6; // Approximate width of the text in pixels
    int16_t displayWidth = SCREEN_WIDTH;

    while (x + textWidth > 0) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(x, y);
        display.print(message);
        display.display();
        delay(100);
        x -= 2; // Scroll speed
    }
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print(message);
    display.display();
}

void tareDisplay() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Tare Button Pressed");
    display.display();
    delay(2000);
}