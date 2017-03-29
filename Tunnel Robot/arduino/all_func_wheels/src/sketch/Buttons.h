// stop and go buttons

#ifndef ALL_FUNC_WHEELS_BUTTONS_H
#define ALL_FUNC_WHEELS_BUTTONS_H

#include "Arduino.h"
#include "MiscDefinitions.h"

#define GoPin 19  // Go button - INTERRUPT PIN
#define StopPin 18  // Stop button - INTERRUPT PIN

class Buttons
{
public:
    // indexes
    static const size_t GO = 0;
    static const size_t STOP = 1;
    static const size_t BUTTON_COUNT = 2;

    pin pins[BUTTON_COUNT];
  
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
        pins[GO] = GoPin;
        pins[STOP] = StopPin;
        state[GO] = '0';
        state[STOP] = '0';
        lastInterruptCallTime[GO] = 0 - MAX_TIME_PRESS;
        lastInterruptCallTime[GO] = 0 - MAX_TIME_PRESS;
        pinMode(pins[GO], INPUT);
        pinMode(pins[STOP], INPUT);
        attachInterrupt(digitalPinToInterrupt(pins[GO]), *(interruptPointer[GO]), CHANGE);
        attachInterrupt(digitalPinToInterrupt(pins[STOP]), *(interruptPointer[STOP]), CHANGE);
    }

    void lowInterrupt(const size_t& whichButton) {
        lastInterruptCallTime[whichButton] = millis();
    }

    void highInterrupt(const size_t& whichButton) {
        long time = millis();
        
        if (time - lastInterruptCallTime[whichButton] > MIN_TIME_PRESS &&
            time - lastInterruptCallTime[whichButton] < MAX_TIME_PRESS)
        {
            state[whichButton] = '1';
        }
        else  // this wasn't a real button press
        {
            // reset last button down
            lastInterruptCallTime[whichButton] = 0 - MAX_TIME_PRESS;
        }
    }

    char getGoState() const {
        return state[GO];
    }

    char getStopState() const {
        return state[STOP];
    }
};

#endif //ALL_FUNC_WHEELS_BUTTONS_H
