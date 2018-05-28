#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define PRODUCTION true
#define USE_DISPLAY false

#include "DallasTemperature.h"

#define DS18B20_PRECISION 11

#if PRODUCTION
#define DS18B20_INTERVAL 15000 // 15 seconds
#else
#define DS18B20_INTERVAL 15000 // 5000 // 5 seconds
#endif

// sensor address for internal sensor
extern const DeviceAddress DS18B20_INSIDE;

// sensor address for external sensor
extern const DeviceAddress DS18B20_OUTSIDE;

// calibration for external sensor
#define DS18B20_OUTSIDE_LO 1.4  // reference 0.01°C
#define DS18B20_OUTSIDE_HI 98.8 // reference 100°C

#endif
