// stop and go buttons
// static class, no instance

#ifndef ALL_FUNC_WHEELS_BUTTONS_H
#define ALL_FUNC_WHEELS_BUTTONS_H

#include "Arduino.h"

#define GoPin 18  // Go button - INTERRUPT PIN
#define StopPin 19  // Stop button - INTERRUPT PIN

class Buttons
{
public:  // private
    static const int SEPARATE_INTERRUPT_CALLS = 100;

    static volatile char goState;
    static volatile char stopState;

    static long lastInterruptCallTimeGo;
    static long lastInterruptCallTimeStop;

public:
    static void init() {
        goState = '0';
        stopState = '0';
        pinMode(GoPin, INPUT);
        pinMode(StopPin, INPUT);
        attachInterrupt(digitalPinToInterrupt(GoPin), goInterrupt, RISING);
        attachInterrupt(digitalPinToInterrupt(StopPin), stopInterrupt, RISING);

        attachInterrupt(digitalPinToInterrupt(GoPin), goDown, FALLING);
        attachInterrupt(digitalPinToInterrupt(StopPin), stopDown, FALLING);
    }

    static void goInterrupt() {
        long time = millis();

        if (time - lastInterruptCallTimeGo > SEPARATE_INTERRUPT_CALLS)
            goState = '1';

        lastInterruptCallTimeGo = time;
    }

    static void stopInterrupt() {
        long time = millis();

        if (time - lastInterruptCallTimeStop > SEPARATE_INTERRUPT_CALLS)
            stopState = '1';

        lastInterruptCallTimeStop = time;
    }

    static char getGoState() const {
        return goState;
    }

    static char getStopState() const {
        return stopState;
    }

    static void goDown() {
        Serial.println("go down");
    }

    static void stopDown() {
        Serial.println("stop down");
    }
};

#endif //ALL_FUNC_WHEELS_BUTTONS_H
