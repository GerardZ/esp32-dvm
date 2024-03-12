#pragma once

#include <Arduino.h>

#define CLOCK 2              // TCL7135 pin22 CLKIN     => GIPO_2
#define STB 4                // TCL7135 pin26 -STB      => GPIO_4
#define RHLD 5               // TCL7135 pin25 RUN/-HOLD => GPIO_5
//#define ICL_ClockRate 240000 // vary between 100-480 kHz
#define ICL_ClockRate 100000 // vary between 100-480 kHz

#define adB1 35
#define adB2 34
#define adB4 26
#define adB8 27
#define adD5 32
#define polarity 25

const int16_t digitMultiply[] = {0, 1, 10, 100, 1000, 10000}; // be aware we are not using index 0

void initTCL7135();
void initTCL7135(void (*TclMeasurementCallback)(int16_t));
void doTCL7135();
int16_t getTCL7135();



