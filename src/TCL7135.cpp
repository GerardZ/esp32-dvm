/*

TCL7135 (ICL7135) interfacing.

This a a 4.5 digit AD-converter purposed for voltmeters and is known for a very reliable, steady conversion.

The chip uses intergration for measuring and makes data available in 5 multiplexed digits, where the digit data is 
in BCD and there are 5 D-lines, one per digit. On top of this, there is a /STB pin which gets low when data for that digit 
is valid.
We use that to read results from the AD.

We use the /STB to trigger an interrupt and scan D5 for synchronisation and B-lines for the value of one digit.

USAGE:
    in setup() call initTCL7135() or initTCL7135(callback) to use with callback.

    Then you can call getTCL7135() to simply get the latest measurement or call doTCL7135() in the loop and when there`s a new
    measurement it will call the callback function.
    If you do no wish to use the callback construction you can poll by calling getTCL7135() which simply returns the last 
    valid measurement.

TODO:
    UnderVoltage and overvoltage ?

*/

#include "TCL7135.h"

void (*_TclMeasurementCallback)(int16_t); // holds messagereceived callback

volatile uint8_t currentDigit;
volatile uint8_t digitIndex;
volatile bool ready;
volatile int16_t value;      // though it is 5 digits, we can use int16, we`re only counting +-19999
volatile int16_t validValue; // though it is 5 digits, we can use int16, we`re only counting +-19999

void setupTclPins()
{
    pinMode(STB, INPUT); // Interrupt - ESP32 GPIO_4

    pinMode(adB1, INPUT);
    pinMode(adB2, INPUT);
    pinMode(adB4, INPUT);
    pinMode(adB8, INPUT);
    pinMode(adD5, INPUT);

    pinMode(polarity, INPUT);
}

void setupTclClock()
{
    pinMode(CLOCK, OUTPUT);         // GPIO_2 as Output
    ledcAttachPin(CLOCK, 0);        // GPIO_2 attached to PWM Channel 0
    ledcSetup(0, ICL_ClockRate, 2); // Channel 0 , freq 480 KHz , 2 bit resolution
    ledcWrite(0, 2);                // Enable frequency with duty cycle 50%
}

void resetTcl()
{
    pinMode(RHLD, OUTPUT);   // RUN DVM   - ESP32 GIPO_5
    digitalWrite(RHLD, LOW); // Reset DVM TLC7135
    delay(100);
    digitalWrite(RHLD, HIGH); // Run DVM TLC7135
}

void IRAM_ATTR ISR_Measure()  // Interrupt routine driven by /STB of the ICL7135, this will pulse on every digit when data for that digit on B-pins is ready
{
    if (digitalRead(adD5)) // start from the 'beginning' = 5, we use only D5 and count with STB for subsequent bytes, see data sheet.
    {
        digitIndex = 5;
        ready = false;
        value = 0;
    }

    currentDigit = 0;       // read in digitBits from 7135`s B-lines and compose them to a byte
    if (digitalRead(adB1))
        currentDigit += 1;
    if (digitalRead(adB2))
        currentDigit += 2;
    if (digitalRead(adB4))
        currentDigit += 4;
    if (digitalRead(adB8))
        currentDigit += 8;

    value += currentDigit * digitMultiply[digitIndex];
    digitIndex--;

    if (digitIndex == 0)
    {
        if (!digitalRead(polarity))
            value = -value;
        validValue = value;
        ready = true;
    }
}

void EnableInterrupt()
{
    pinMode(STB, INPUT_PULLUP);
    attachInterrupt(STB, ISR_Measure, FALLING); // External Interrupt /STB pin
}

void initTCL7135(){
setupTclPins();
    setupTclClock();
    EnableInterrupt();
    resetTcl();
}

void initTCL7135(void (*TclMeasurementCallback)(int16_t))
{
    _TclMeasurementCallback = TclMeasurementCallback;
    initTCL7135();
}

void doTCL7135()        // use this with callback, it will call the callback function when there`s a new reading.
{ 
    if (ready)
    {
        if (_TclMeasurementCallback)
            _TclMeasurementCallback(validValue);
    }
    ready = false;
}

int16_t getTCL7135()    // use this for polling, it just returns the latest measurement.
{ 
    return validValue;
}
