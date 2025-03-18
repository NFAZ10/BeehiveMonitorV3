#ifndef WEBSERIALSETUP_H
#define WEBSERIALSETUP_H

#include <Arduino.h>
#include <WebSerial.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ESPDash.h>

// Declare as extern to avoid multiple definitions
extern AsyncWebServer server;


void webserial();      // Function to initialize WebSerial
void Dashloop();       // Function to update Card values and send updates to Dashboard



#endif // WEBSERIALSETUP_H
