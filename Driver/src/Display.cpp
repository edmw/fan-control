#include <Arduino.h>

#include "Display.h"

#include "OS/OS.h"

#include <Wire.h>

#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
#define DISPLAY_I2CADDR 0x3c

#define REALM F("Display")

///////////////////////////////////////////////////////////////////////////////////////////////////

class Display::Implementation {
    Display * const display;

public:
    bool connected;

    SSD1306AsciiAvrI2c oled;

    Implementation(Display *d) : display(d) {
    }

    bool begin(void) {
        Wire.begin();
        Wire.beginTransmission(DISPLAY_I2CADDR);
        connected = (Wire.endTransmission () == 0);
        os.notification.info(REALM, F("Begin ..."), connected ? F("Connected") : F("Not connected"));

        if (connected) {
            oled.begin(&Adafruit128x32, DISPLAY_I2CADDR);
            oled.setFont(fixed_bold10x15);
        }
        return connected == true;
    }

};

///////////////////////////////////////////////////////////////////////////////////////////////////

Display::Display()
    : impl { new Implementation(this) } {
}

Display::~Display() {
    delete impl;
}

bool Display::begin(bool production = true) {
    return impl->begin();
}

bool Display::loop() {
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void Display::displayStartup(void) {
    if (impl->connected) {
        impl->oled.clear();
        impl->oled.print(F("Starting ..."));
    }
}

void Display::displayMeasurement(float temperature, s2s_result_t save) {
    if (impl->connected) {
        impl->oled.clear();
        impl->oled.print(F("Temp: "));
        impl->oled.println(String(temperature, 1));
        impl->oled.print(F("Save: "));
        switch (save) {
        case no_wifi_shield:
            impl->oled.println(F("No WiFi shield"));
            break;
        case no_wifi_connection:
            impl->oled.println(F("No WiFi connection"));
            break;
        case ok:
            impl->oled.println(F("OK"));
            break;
        default:
            impl->oled.println(F("?"));
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
