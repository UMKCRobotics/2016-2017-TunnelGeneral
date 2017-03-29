// stop and go buttons

#ifndef ALL_FUNC_WHEELS_BUTTONS_H
#define ALL_FUNC_WHEELS_BUTTONS_H

#include "Arduino.h"

#define GoPin 18  // Go button - INTERRUPT PIN
#define StopPin 19  // Stop button - INTERRUPT PIN

class Buttons
{
public:
    // indexes
    static const size_t GO = 0;
    static const size_t STOP = 1;
    static const size_t BUTTON_COUNT = 2;
  
public:  // private
    static const int MIN_TIME_PRESS = 250;
    static const int MAX_TIME_PRESS = 1001;

    volatile char state[BUTTON_COUNT];

    long lastInterruptCallTime[BUTTON_COUNT];

    void (*interruptPointer[BUTTON_COUNT])();

public:
    Buttons(void (*_goInterrupt)(), void (*_stopInterrupt)()) {
        interruptPointer[GO] = _goInterrupt;
        interruptPointer[STOP] = _stopInterrupt;
    }

    void init() {
        state[GO] = '0';
        state[STOP] = '0';
        pinMode(GoPin, INPUT);
        pinMode(StopPin, INPUT);
        attachInterrupt(digitalPinToInterrupt(GoPin), *(interruptPointer[GO]), CHANGE);
        attachInterrupt(digitalPinToInterrupt(StopPin), *(interruptPointer[STOP]), CHANGE);
    }

    void interrupt(const size_t& whichButton) {
        long time = millis();

        if (time - lastInterruptCallTime[whichButton] > MIN_TIME_PRESS &&
            time - lastInterruptCallTime[whichButton] < MAX_TIME_PRESS)
            state[whichButton] = '0';  // TODO: disabled - change back to '1'

        lastInterruptCallTime[whichButton] = time;
    }

    char getGoState() const {
        return state[GO];
    }

    char getStopState() const {
        return state[STOP];
    }
};

#endif //ALL_FUNC_WHEELS_BUTTONS_H
