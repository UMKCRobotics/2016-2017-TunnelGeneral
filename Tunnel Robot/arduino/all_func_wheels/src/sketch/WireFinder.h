// manage sensor to find wire

#ifndef ALL_FUNC_WHEELS_WIREFINDER_H
#define ALL_FUNC_WHEELS_WIREFINDER_H

#include "Arduino.h"

#define EMF_PIN A0

class WireFinder
{
public:  // private
    static const int SAMPLE_COUNT = 300;

public:
    static int read() {

        long sum = 0;
        for (int i = SAMPLE_COUNT; i > 0; --i) {
            sum += analogRead(EMF_PIN);
        }

        return sum / SAMPLE_COUNT;
    }
};

#endif //ALL_FUNC_WHEELS_WIREFINDER_H
