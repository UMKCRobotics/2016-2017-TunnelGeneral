#include <Arduino.h>
#include "Adafruit_GFX.h"
#include "Adafruit_NeoMatrix.h"
#include "Adafruit_NeoPixel.h"
void setup();
void loop();
#line 1 "src/sketch.ino"
//#include "Adafruit_GFX.h"
//#include "Adafruit_NeoMatrix.h"
//#include "Adafruit_NeoPixel.h"
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

#define PIN 6

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);


void setup() {
  matrix.begin();
  matrix.show(); //set all to off
  matrix.setPixelColor(0, 155, 155, 0);
  matrix.show();
}

void loop() { 
  
}
