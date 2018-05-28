#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <Arduino.h>

#include "save2server.h"

class Display {
    class Implementation;

public:
    Display();
    ~Display();

    bool begin(bool production);

    bool loop(void);

    void displayStartup(void);
    void displayMeasurement(float temperature, s2s_result_t save);

private:
    Display(const Display&);
    Display& operator=(const Display&);

    Implementation *impl;
};

#endif
