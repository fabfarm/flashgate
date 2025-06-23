#include "WifiUtil.h"
#include <WiFi.h>
#include "Util.h"

// WIFI credentials
const char *ssid = "fabfarm";
const char *password = "imakestuff";

// Timeout settings
const int maxAttempts = 30; 

void setupWifi()
{
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  
  int attempts = 0;
  
  // attempt to connect to WiFi with timeout
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts)
  {
    wait(1000);
    attempts++;
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("\nFailed to connect to WiFi :()");
  }
}