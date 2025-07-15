#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "Config.h"
#include "SensorLogic.h"

AsyncWebServer server(80);

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

  // Register dynamic endpoint first!
  server.on("/light", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = String("{\"value\":") + getLightValue()+ "}";
    request->send(200, "application/json", json);
  });

  // Then serve static files
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  server.begin();
}
