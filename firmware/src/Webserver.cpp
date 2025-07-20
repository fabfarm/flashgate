#include <Arduino.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "Config.h"
#include "PatternDetection.h"

// Forward declarations for manual gate control
extern void manualOpenGate();
extern void manualCloseGate();
extern void testServoMovement();

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

  // Manual gate control endpoints
  server.on("/gate/open", HTTP_POST, [](AsyncWebServerRequest *request){
    manualOpenGate();
    request->send(200, "application/json", "{\"status\":\"gate opened\"}");
  });

  server.on("/gate/close", HTTP_POST, [](AsyncWebServerRequest *request){
    manualCloseGate();
    request->send(200, "application/json", "{\"status\":\"gate closed\"}");
  });

  // Servo test endpoint
  server.on("/servo/test", HTTP_POST, [](AsyncWebServerRequest *request){
    testServoMovement();
    request->send(200, "application/json", "{\"status\":\"servo test executed\"}");
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

  

