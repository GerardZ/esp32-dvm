#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#include "TCL7135.h"
#include <apphtml.h>


void ISR_Measure();
void PrintVoltage();

#include <Wire.h>

void MeasurementReady(int16_t value);
