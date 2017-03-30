#include "Arduino.h"

#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif
//test LED pins
#define LED1 22
#define LED2 23
//EMF PINS
#define EMF1 A0

void setup() {
  //initialize led pins
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(EMF1, INPUT);
  //start serial
  Serial.begin(115200);
  //start serial1 to motor controller
  //Serial1.begin(115200);
  //send READY byte
  Serial.write('1');
  
}

void loop() {
  getEMFReading();
  Serial.write('\n');
}


//START OF SENSOR STUFF
void sensorReport(String name, int number) {
  char tbs[16];

  sprintf(tbs, " %4d ", number);

  Serial.write(' ');
  Serial.print(name);
  // Serial.write(' ');
  Serial.print(tbs);
  // Serial.write(' ');
}

int getReadingAverage(int whichPin) {
  // average of lots of readings
  const long sampleCount = 200;
  long total = 0;
  for (int i = sampleCount; i > 0; --i) {
    total += analogRead(whichPin);
  }

  return total / sampleCount;
}

int getReadingMax(int whichPin) {
  // average of lots of readings
  const long sampleCount = 200;
  long maxim = 0;
  for (int i = sampleCount; i > 0; --i) {
    maxim = max(maxim, analogRead(whichPin));
  }

  return maxim;
}

void getEMFReading() {
  int emf1 = getReadingAverage(EMF1);

  //check right
  sensorReport("emf1", emf1);
}

//END OF SENSOR STUFF
