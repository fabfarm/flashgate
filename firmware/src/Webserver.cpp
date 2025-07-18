#include <Arduino.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "Config.h"
#include "PatternDetection.h"

AsyncWebServer server(80);
AsyncEventSource events("/events");


void setupWebServer() {
  if (!LittleFS.begin(true)) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  Serial.println("LittleFS mounted. Files:");
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  while (file) {
    Serial.println(file.name());
    file = root.openNextFile();
  }
  Serial.println("LittleFS files listed.");

  // Register dynamic endpoints
  server.on("/light", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = String("{\"value\":") + getLightValue() + "}";
    request->send(200, "application/json", json);
  });

  // Then serve static files
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
  server.addHandler(&events);
  server.begin();
}

void sendData(String eventName, String data) {
  if (events.count() == 0) {return; } // only send if clients connected
  String json = "{\"" + eventName + "\":\"" + String(data) + "\"}";
  events.send(json.c_str(), "message", millis());
}

  

