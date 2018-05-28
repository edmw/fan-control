///////////////////////////////////////////////////////////////////////////////////////////////////
// FAN control driver for Arduino
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>

#define REALM F("Driver")

#include "OS/OS.h"
extern OS os;

#include "Driver.h"
#include "private.h"

Driver driver;

#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);

#include "save2server.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

String deviceaddress_to_hexstring(DeviceAddress address) {
    char hex[16+1];
    for (uint8_t i = 0; i < 8; i++) {
        snprintf(hex + i*2, 3, "%02x", address[i]);
    }
    return String(hex);
}

inline float ds18b20_calibrate(float t, float lo, float hi) {
    if (isnan(t)) {
      return t;
    }
    return ((((t - lo) * 99.99) / (hi - lo)) + 0.01);
}

void ds18b20_begin() {
    ds18b20.begin();

    uint8_t n = ds18b20.getDeviceCount();
    for(uint8_t i = 0; i < n; i++) {
        os.notification.info(REALM, F("DS18B20 device at"), String(i, DEC));
        DeviceAddress address;
        if (ds18b20.getAddress(address, i)) {
            os.notification.info(REALM, F("DS18B20 device address"),
                deviceaddress_to_hexstring(address)
            );
            ds18b20.setResolution(address, DS18B20_PRECISION);
        }
        else {
            os.notification.info(REALM, F("DS18B20 ghost device"));
        }
    }
}

void ds18b20_measure(void *p) {
    driver.signal.on();

    ds18b20.requestTemperatures();
    delay(100);

    driver.measurement = ds18b20_calibrate(
        ds18b20.getTempC(DS18B20_INSIDE),
        0.01,
        100.0
    );
    if (isnan(driver.measurement)) {
        os.notification.warn(REALM, F("Failed to read from internal DS18B20 sensor!"));
    }

    driver.signal.off();
}

void setup() {
    os.begin(PRODUCTION);

    driver.signal.begin(PRODUCTION);

    #if USE_DISPLAY
    driver.display.begin(PRODUCTION);
    driver.display.displayStartup();
    #endif

    save2server_setup();

    driver.fan.begin(PRODUCTION);
    driver.fan_running.begin(PRODUCTION);

    ds18b20_begin();

    driver.temperature_high.begin(PRODUCTION);
    driver.temperature_cool.begin(PRODUCTION);

    driver.measure.attach_ms(DS18B20_INTERVAL, ds18b20_measure, 0);
    ds18b20_measure(0);
}

s2s_result_t save2server_result = no;

void loop() {

    // get current temperature and control fan
    if ( // new measurement differs from current temperature
        (!isnan(driver.measurement) &&  isnan(driver.temperature)) ||
        ( isnan(driver.measurement) && !isnan(driver.temperature)) ||
        (fabs(driver.temperature - driver.measurement) > 0.01)
    ) {
        float t = driver.measurement;
        if (isnan(t) || (t < -10)) {
            os.notification.info(REALM, F("Temperature (°C):"), F("N/A"));
            // measurement not sensible, start fan and blink leds
            driver.fan.on();
            driver.fan_running.on();
            driver.temperature_high.blinking();
        }
        else {
            os.notification.info(REALM, F("Temperature (°C):"), String(t, 2));
            if (t >= TEMPERATURE_HIGH) {
                // temperature is above limit,
                // start fan and set leds (hi on, cool off)
                driver.fan.on();
                driver.fan_running.on();
                driver.temperature_high.on();
                driver.temperature_cool.off();
            }
            else {
                // temperature is not above limit but also not below lower bound,
                // keep fan running and set leds (hi off, cool on)
                driver.temperature_high.off();
                if (t >= TEMPERATURE_COOL && driver.fan.isOn()) {
                    driver.temperature_cool.on();
                }
            }
            if (t < TEMPERATURE_COOL) {
                // temperature is below lower bound,
                // stop fan and set leds (all off)
                driver.fan.off();
                driver.fan_running.off();
                driver.temperature_high.off();
                driver.temperature_cool.off();
            }
        }

        driver.temperature = t;

        #if USE_DISPLAY
        driver.display.displayMeasurement(driver.temperature, save2server_result);
        #endif
    }

    // save current temperature to server
    s2s_result_t save = save2server(driver.temperature, driver.fan.isOn());
    if (save != skip) {
        save2server_result = save;
        #if USE_DISPLAY
        driver.display.displayMeasurement(driver.temperature, save2server_result);
        #endif
    }

    // do the loop

    driver.measure.update();
    driver.fan.loop();

    driver.temperature_high.loop();
    driver.temperature_cool.loop();
    driver.fan_running.loop();

    os.yield();
}
