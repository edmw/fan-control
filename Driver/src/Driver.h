#ifndef __DRIVER_H__
#define __DRIVER_H__

///////////////////////////////////////////////////////////////////////////////////////////////////
// FAN control driver for Arduino
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>

#include "global.h"

#define TEMPERATURE_HIGH 35
#define TEMPERATURE_COOL 28 // < TEMPERATURE_LIMIT!

#if defined(__AVR_ATmega328P__)
#define DS18B20_PIN 2
#define FAN_PIN 3
#define FAN_LED_PIN 6
#define TEMP_HIGH_LED_PIN 4
#define TEMP_COOL_LED_PIN 5
#else
#error "Driver configuration available for ATmega328 only!"
#endif

#include "OS/OS.h"

#include "Display.h"

class Driver final {

public:
    Signaling signal;

    #if USE_DISPLAY
    Display display;
    #endif

    Ticker measure;

    Signaling temperature_high;
    Signaling temperature_cool;

    Signaling fan;
    Signaling fan_running;

    float measurement = 0;
    float temperature = 0;

    Driver() :
        signal(
            LED_BUILTIN, HIGH
        ),
        #if USE_DISPLAY
        display(
        ),
        #endif
        measure(
        ),
        temperature_high(
            TEMP_HIGH_LED_PIN, HIGH
        ),
        temperature_cool(
            TEMP_COOL_LED_PIN, HIGH
        ),
        fan(
            FAN_PIN, HIGH
        ),
        fan_running(
            FAN_LED_PIN, HIGH
        )
    {}

};

#endif
