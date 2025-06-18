#include "Config.h"
#include "WifiUtils.h"
#include "HttpServer.h"
#include "FileSystem.h"
#include "movingAvg.h"
#include "Arduino.h"

#define LIGHT_SENSOR_PIN 34 // ESP32 pin GIOP36 (ADC0)
#define EXTERNAL_LED 21
#define INTERNAL_LED 2
#define N 15
#define SHORT_PULSE_WINDOW 6
#define LONG_PULSE_WINDOW 9
#define THRESHOLD 300.0

// Function declarations
void read_and_detect_pulse();

uint8_t byte_index = 0;
uint16_t history[N];

void create_samples(uint16_t sample)
{
  uint16_t previous = history[byte_index]; // get oldest from buffer
  history[byte_index] = sample;            // insert newest
  byte_index++;                            // move pointer circularly

  if (byte_index >= N)
  {
    byte_index = 0;
  }
}

// Function to compare averages of two sets of N elements each
int detectPulses(int size)
{
  // Calculate the average of the older elements (first part of buffer)
  double avg1 = 0;
  int start1 = (byte_index + N - size) % N;
  for (int i = 0; i < size; i++)
  {
    avg1 += history[(start1 + i) % N];
  }
  avg1 /= (double)size;

  // Calculate the average of the recent elements (last 'size' elements)
  double avg2 = 0;
  for (int i = 0; i < size; i++)
  {
    int idx = (byte_index + N - size + i) % N;
    avg2 += history[idx];
  }
  avg2 /= (double)size;
  // Serial.printf("Avg1:%.3f\n", avg1);
  // Serial.printf("Avg2:%.3f\n", avg2);

  double diff = avg2 - avg1; // Compare recent vs older
  // Serial.printf("Diff:%.3f\n", diff);

  // Compare averages
  if (diff >= THRESHOLD)
    return 1;
  else
    return 0;
}

void setup()
{
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
  setupFileSystem();
  setupWifi();
  setupHttpServer();
  pinMode(EXTERNAL_LED, OUTPUT);
  pinMode(INTERNAL_LED, OUTPUT);
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  digitalWrite(EXTERNAL_LED, LOW);
  digitalWrite(INTERNAL_LED, LOW);
}

void loop()
{
  read_and_detect_pulse();
}

void read_and_detect_pulse()
{ 
  // reads the input on analog pin (value between 0 and 4095)
  int sensorData = analogRead(LIGHT_SENSOR_PIN);
  uint16_t sample = (uint16_t)sensorData;
  unsigned long startTime = millis();

  create_samples(sample);
  
  // Change to  LONG_PULSE_WINDOW to detect long pulse
  if (detectPulses(SHORT_PULSE_WINDOW) == 1)
  {
    Serial.printf("Short Pulse Detected\n");
    digitalWrite(EXTERNAL_LED, HIGH); // turn on LED
    while (millis() - startTime < 100)
    {
    }
  }
  else
  {
    digitalWrite(EXTERNAL_LED, LOW); // turn off LED
    digitalWrite(INTERNAL_LED, LOW);
  }

  while (millis() - startTime < 50)
  {
    // Do nothing, just wait
  }

  // Probably need a state machine to handle the flash code pattern
  // See https://forum.arduino.cc/t/how-to-detect-a-pattern/410125/4 
  // and https://github.com/jrullan/StateMachine

  // bool codeMatched = false;
  // bool codeIndex = 0; 
  // if (!codeMatched && codeIndex < flashCodePattern.size())
  //     {
  //       if ((flashCodePattern[codeIndex] && detectPulses(LONG_PULSE_WINDOW) == 1) ||
  //           (flashCodePattern[codeIndex] && detectPulses(SHORT_PULSE_WINDOW) == 1))
  //       {
  //         codeIndex++;
  //         if (codeIndex == flashCodePattern.size())
  //         {
  //           codeMatched = true;
  //           Serial.println("Code Matched!");
  //           // Do something when code is matched
  //         }
  //       }
  //       else
  //       {
  //         codeIndex = 0; // reset code index if code doesn't match
  //       }
  //     }

}
