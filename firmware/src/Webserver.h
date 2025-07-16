#pragma once
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

extern AsyncWebServer server;
void setupWebServer();
void sendData(String eventName, String data);


