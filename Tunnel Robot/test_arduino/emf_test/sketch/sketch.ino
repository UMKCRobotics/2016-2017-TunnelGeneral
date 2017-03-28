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

int getIRReading(int whichPin) {
  // average of lots of readings
  const long sampleCount = 200;
  long total = 0;
  for (int i = sampleCount; i > 0; --i) {
    total += analogRead(whichPin);
  }

  return total / sampleCount;
}

String getEMFReading() {
  int emf1 = getIRReading(EMF1);

  String report = "";
  //report format: right,front,left,back
  int threshold = 190; //set this to something reasonable
  //check right
  sensorReport("emf1", emf1);
  return report;
}

int getEMFReading(int port) {
  int count = 0;
  long int sum = 0;
  for (int i = 0; i < 500; i++) {
    int reading = analogRead(port);
    if (reading != 0) {
      count++;
      //sum += reading*reading;
      sum += pow(reading,2);
      //sum += abs(reading);
    }
  }
  long int average = 0;
  if (count > 0)
    average = sum/count;
  return average;
}
//END OF SENSOR STUFF
