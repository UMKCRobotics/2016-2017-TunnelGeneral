#include "Arduino.h"

#define GO_PIN 18

void interruptFunction() {
    Serial.println("interrupt called");
}

void setup() {
    attachInterrupt(digitalPinToInterrupt(GO_PIN), interruptFunction, CHANGE);
}

void loop() {

}
