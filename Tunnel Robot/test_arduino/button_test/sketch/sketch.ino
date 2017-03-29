#include "Arduino.h"

#define GO_PIN 18

void interruptFunction() {
    Serial.print("interrupt called ");
    int pinRead = digitalRead(GO_PIN);

    Serial.println(pinRead);
}

void setup() {
    Serial.begin(115200);
    pinMode(GO_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(GO_PIN), interruptFunction, CHANGE);
}

void loop() {
    Serial.println("loop");
    delay(2000);
}
