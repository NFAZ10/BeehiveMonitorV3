#include <Arduino.h>
#include <WiFi.h>
#include <ESPDash.h>
#include "webserialsetup.h"
#include "variables.h"  

extern AsyncWebServer server;  // Define WebServer instance

ESPDash dashboard(&server);
Card temperature(&dashboard, TEMPERATURE_CARD, "Temperature", "°C");
Card humidity(&dashboard, HUMIDITY_CARD, "Humidity", "%");




void Dashloop() {
    /* Update Card Values */
    temperature.update((int)random(0, 50));
    humidity.update((int)random(0, 100));
  
    /* Send Updates to our Dashboard (realtime) */
    dashboard.sendUpdates();
  
    /* 
      Delay is just for demonstration purposes in this example,
      Replace this code with 'millis interval' in your final project.
    */
    delay(3000);
  }