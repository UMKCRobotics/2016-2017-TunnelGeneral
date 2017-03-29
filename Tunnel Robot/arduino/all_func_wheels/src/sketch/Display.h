// matrix display and seven segment

#ifndef ALL_FUNC_WHEELS_DISPLAY_H
#define ALL_FUNC_WHEELS_DISPLAY_H

#include "Arduino.h"
#include "Adafruit_GFX.h"
#include "Adafruit_NeoMatrix.h"
#include "Adafruit_NeoPixel.h"

// test LED pins
#define LED1 22
#define LED2 23

// 8x8 matrix
#define MATRIX_PIN 6

// 7 segment pins
#define SEVEN_SEGMENT_DATA 7    // serial pin
#define SEVEN_SEGMENT_LATCH 9   // register clock pin
#define SEVEN_SEGMENT_CLK 8     // serial clock pin

// setup matrix
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, MATRIX_PIN,
                                                          NEO_MATRIX_TOP + NEO_MATRIX_RIGHT +
                                                          NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
                                                          NEO_GRB + NEO_KHZ800);

// digit representations
static const int digits[10] = {190, 6, 218, 206, 102, 236, 252, 134, 254, 238};

class Display
{
public:  // private
    static const uint16_t MATRIX_READY_LIGHT_PIXEL = 56;

public:
    static void init() {
        // initialize led pins
        pinMode(LED1, OUTPUT);
        pinMode(LED2, OUTPUT);

        // initialize matrix
        matrix.begin();
        matrix.setBrightness(20);
        matrix.show();  // set all to off

        // setup 7 digit display and clear it
        pinMode(SEVEN_SEGMENT_LATCH, OUTPUT);
        pinMode(SEVEN_SEGMENT_CLK, OUTPUT);
        pinMode(SEVEN_SEGMENT_DATA, OUTPUT);
        clearDigit();
    }

    /**
     * 7 segment for displaying number on die
     * @param dig the number to display
     */
    static void displayDigit(int dig) {
        for (int i = digits[dig]; i <= digits[dig] + 1; i++) {
            digitalWrite(SEVEN_SEGMENT_LATCH, HIGH);
            int number = i;
            shiftOut(SEVEN_SEGMENT_DATA, SEVEN_SEGMENT_CLK, MSBFIRST, ~(char) number);  // digitOne
            digitalWrite(SEVEN_SEGMENT_LATCH, LOW);
            delay(1);
        }
    }

    static void clearDigit() {
        digitalWrite(SEVEN_SEGMENT_LATCH, HIGH);
        int number = 0;
        shiftOut(SEVEN_SEGMENT_DATA, SEVEN_SEGMENT_CLK, MSBFIRST, ~(char) number); // digitOne
        digitalWrite(SEVEN_SEGMENT_LATCH, LOW);
        delay(1);
    }

    static void setReadyLight() {
        matrix.setPixelColor(MATRIX_READY_LIGHT_PIXEL, 255, 255, 0);  // TODO: confirm this color is yellow
        matrix.show();
    }

    static void setToOT(uint16_t index) {
        if (index != MATRIX_READY_LIGHT_PIXEL)  // don't change the ready light
        {
            matrix.setPixelColor(index, 255, 0, 0);
            matrix.show();
        }
    }

    static void setToDE(uint16_t index) {
        if (index != MATRIX_READY_LIGHT_PIXEL)  // don't change the ready light
        {
            matrix.setPixelColor(index, 0, 255, 255);  // TODO: confirm this color is blue
            matrix.show();
        }
    }

    static void setToEM(uint16_t index) {
        if (index != MATRIX_READY_LIGHT_PIXEL)  // don't change the ready light
        {
            matrix.setPixelColor(index, 0, 0, 0);
            matrix.show();
        }
    }
};

#endif //ALL_FUNC_WHEELS_DISPLAY_H
