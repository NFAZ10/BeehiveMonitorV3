#include "webserialsetup.h"

AsyncWebServer server(80);  // Define WebServer instance


void webserial() {
    WebSerial.begin(&server); // Start WebSerial
    server.begin();           // Start WebServer
    WebSerial.println("WebSerial initialized!");
}

