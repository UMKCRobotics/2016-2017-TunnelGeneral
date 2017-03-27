// stop and go buttons

#ifndef ALL_FUNC_WHEELS_BUTTONS_H
#define ALL_FUNC_WHEELS_BUTTONS_H

#include "Arduino.h"

#define GoPin 18  // Go button - INTERRUPT PIN
#define StopPin 19  // Stop button - INTERRUPT PIN

class Buttons
{
public:  // private
    volatile char goState;
    volatile char stopState;

    void (*goInterrupt)();
    void (*stopInterrupt)();

public:
    Buttons(void (*_goInterrupt)(), void (*_stopInterrupt)()) {
        goInterrupt = _goInterrupt;
        stopInterrupt = _stopInterrupt;
    }

    void init() {
        goState = '0';
        stopState = '0';
        pinMode(GoPin, INPUT);
        pinMode(StopPin, INPUT);
        attachInterrupt(digitalPinToInterrupt(GoPin), *goInterrupt, CHANGE);
        attachInterrupt(digitalPinToInterrupt(StopPin), *stopInterrupt, CHANGE);
    }

    void goInterrupt() {
        goState = '1';
    }

    void stopInterrupt() {
        // stopState = '1';  // TODO: uncomment when we have a reliable button attached
    }

    const char& getGoState() {
        return goState;
    }

    const char& getStopState() {
        return stopState;
    }
};

#endif //ALL_FUNC_WHEELS_BUTTONS_H
