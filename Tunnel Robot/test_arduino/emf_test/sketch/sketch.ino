#include "Arduino.h"


//test LED pins
#define LED1 22
#define LED2 23
//EMF PINS
#define EMF1 A1

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
  /*
  int reading = getReadingMax(EMF1);
  int readings = 1;
  if(reading > 5)
  {
    Serial.println(reading);
    while(reading > 5)
    {
      reading = getReadingMax(EMF1);
      readings += 1;
    }
    Serial.print(" emf readings ");
    
    Serial.println(analogRead(EMF1));
  }
  */

  // Serial.println(analogRead(EMF1));
  
  //test_timing();
  test_min_max();
  //getEMFReading();
  //Serial.write('\n');
}

void test_timing() {
  long a = millis();
  for (int i = 300; i > 0; --i) {
    analogRead(EMF1);
  }
  Serial.println(millis() - a);
}

void test_min_max() {
  int minRead = 1023;
  int maxRead = 0;
  int reading;
  for (int i = 300; i > 0; --i) {
    reading = analogRead(EMF1);
    if (reading < minRead)
      minRead = reading;
    if (reading > maxRead)
      maxRead = reading;
  }

  Serial.println(maxRead - minRead);
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
  //int emf1 = analogRead(EMF1);
  int emf1 = getReadingMax(EMF1);
  int readings = 1;
  while(emf1 > 5)
  {
    emf1 = getReadingMax(EMF1);
    readings += 1;
  }

  //check right
  sensorReport("emf1", readings);
}

//END OF SENSOR STUFF
