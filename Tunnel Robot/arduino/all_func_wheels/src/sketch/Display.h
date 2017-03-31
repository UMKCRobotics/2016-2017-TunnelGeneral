// matrix display and seven segment

#ifndef ALL_FUNC_WHEELS_DISPLAY_H
#define ALL_FUNC_WHEELS_DISPLAY_H

#include "Arduino.h"
#include "Adafruit_GFX.h"
#include "Adafruit_NeoMatrix.h"
#include "Adafruit_NeoPixel.h"
#include "MiscDefinitions.h"

// test LED pins
#define LED1 22
#define LED2 23

// 8x8 matrix
#define MATRIX_PIN 6

// 7 segment pins
#define ONE_DIGIT 7    // binary digit
#define FOUR_DIGIT 9
#define TWO_DIGIT 8

const pin sevenPins[3] = {9, 8, 7};



// setup matrix
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, MATRIX_PIN,
                                                          NEO_MATRIX_TOP + NEO_MATRIX_RIGHT +
                                                          NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
                                                          NEO_GRB + NEO_KHZ800);

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
        pinMode(FOUR_DIGIT, OUTPUT);
        pinMode(TWO_DIGIT, OUTPUT);
        pinMode(ONE_DIGIT, OUTPUT);
        clearDigit();
    }

    /**
     * 7 segment for displaying number on die
     * @param dig the number to display
     */
    static void displayDigit(int dig) {
        digitalWrite(ONE_DIGIT, HIGH);
        digitalWrite(TWO_DIGIT, HIGH);
        digitalWrite(FOUR_DIGIT, LOW);
    }

    static void clearDigit() {
        digitalWrite(ONE_DIGIT, LOW);
        digitalWrite(TWO_DIGIT, LOW);
        digitalWrite(FOUR_DIGIT, LOW);
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
