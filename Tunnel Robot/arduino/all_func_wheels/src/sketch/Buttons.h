// stop and go buttons

#ifndef ALL_FUNC_WHEELS_BUTTONS_H
#define ALL_FUNC_WHEELS_BUTTONS_H

#include "Arduino.h"

#define GoPin 18  // Go button - INTERRUPT PIN
#define StopPin 19  // Stop button - INTERRUPT PIN

class Buttons
{
public:  // private
    static const int SEPARATE_INTERRUPT_CALLS = 100;
    
    volatile char goState;
    volatile char stopState;

    long lastInterruptCallTimeGo;
    long lastInterruptCallTimeStop;

    void (*goInterruptPointer)();
    void (*stopInterruptPointer)();
    void (*goDownPointer)();
    void (*stopDownPointer)();

public:
    Buttons(void (*_goInterrupt)(), void (*_stopInterrupt)(), void (*_goDown)(), void (*_stopDown)()) {
        goInterruptPointer = _goInterrupt;
        stopInterruptPointer = _stopInterrupt;
        goDownPointer = _goDown;
        stopDownPointer = _stopDown;
    }

    void init() {
        goState = '0';
        stopState = '0';
        pinMode(GoPin, INPUT);
        pinMode(StopPin, INPUT);
        attachInterrupt(digitalPinToInterrupt(GoPin), *goInterruptPointer, RISING);
        attachInterrupt(digitalPinToInterrupt(StopPin), *stopInterruptPointer, RISING);
        attachInterrupt(digitalPinToInterrupt(GoPin), *goDownPointer, FALLING);
        attachInterrupt(digitalPinToInterrupt(StopPin), *stopDownPointer, FALLING);
    }

    void goInterrupt() {
        Serial.println("go up");
        long time = millis();

        if (time - lastInterruptCallTimeGo > SEPARATE_INTERRUPT_CALLS)
            goState = '1';

        lastInterruptCallTimeGo = time;
    }

    void stopInterrupt() {
        long time = millis();

        if (time - lastInterruptCallTimeStop > SEPARATE_INTERRUPT_CALLS)
            stopState = '1';

        lastInterruptCallTimeStop = time;
    }

    void goDown() {
        Serial.println("go down");
    }

    void stopDown() {
        Serial.println("stop down");
    }

    char getGoState() const {
        return goState;
    }

    char getStopState() const {
        return stopState;
    }
};

#endif //ALL_FUNC_WHEELS_BUTTONS_H
