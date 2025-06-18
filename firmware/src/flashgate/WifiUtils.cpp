#include "WifiUtils.h"

const char *ssid = "fabfarm";
const char *password = "imakestuff";

void setupWifi()
{
  // Connect to Wi-Fi with timeout
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  
  int attempts = 0;
  const int maxAttempts = 30; // 30 seconds timeout
  
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts)
  {
    delay(1000);
    attempts++;
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nConnected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("\nFailed to connect to WiFi");
    // Could implement AP mode fallback here
  }
}