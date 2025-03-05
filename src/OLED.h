#ifndef OLED_H
#define OLED_H

#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET    -1  // Define OLED_RESET to a default value, e.g., -1 if not used
extern Adafruit_SSD1306 display;
void setupOLED();
void updateOLED();
void clearOLED();
void updateOLEDWithNetworkStatus();
void powerOffOLED();
void powerOnOLED();
void printToOLED(String message);
void tareDisplay();


#endif