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

        int minRead = 1023;
        int maxRead = 0;
        int reading;
        for (int i = SAMPLE_COUNT; i > 0; --i) {
            reading = analogRead(EMF_PIN);
            if (reading < minRead)
                minRead = reading;
            if (reading > maxRead)
                maxRead = reading;
        }

        return maxRead - minRead;
    }
};

#endif //ALL_FUNC_WHEELS_WIREFINDER_H
