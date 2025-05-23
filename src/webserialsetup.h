#ifndef WEBSERIALSETUP_H
#define WEBSERIALSETUP_H

#include <Arduino.h>
#include <WebSerial.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ESPDashPro.h>
#include <NetWizard.h>


// Declare as extern to avoid multiple definitions
extern AsyncWebServer server;



void webserial();      // Function to initialize WebSerial
void wifiSetup();      // Function to setup WiFi
void NWLoop();         // Function to loop NetWizard    
void reconnectWifi();
void disconnectWifi();
void updateEXTTemp(float temp); // Function to update external temperature
void updateEXTHum(float hum);   // Function to update external humidity
void updateweightcard(float dgrams); // Function to update weight
void updatezerocard(float dgrams); // Function to update zero offset
void updatecalcard(float dgrams); // Function to update calibration value
void dashLoop();        // Function to loop DashPro
void attachcallbacks(); // Function to attach callbacks 



#endif // WEBSERIALSETUP_H
