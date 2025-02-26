#ifndef WEBSERIALSETUP_H
#define WEBSERIALSETUP_H

#include <Arduino.h>
#include <WebSerial.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

// Declare as extern to avoid multiple definitions
extern AsyncWebServer server;


void webserial();      // Function to initialize WebSerial



#endif // WEBSERIALSETUP_H
