#include "Adafruit_GFX.h"
#include "Adafruit_NeoMatrix.h"
#include "Adafruit_NeoPixel.h"
#include "Pineapple.h"
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

#define PIN 6

const int serialPIN = 2;           // serial pin
const int registerClockPIN = 4;    // register clock pin
const int serialClockPIN = 3;      // serial clock pin


Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);

String command; //used to store command from serial
String value; //used to store value from serial
String response; //used to store response to main program

Pineapple segmentDisplay; //initialize 7 segment disp library



void setup() {
  matrix.begin();
  matrix.setBrightness(20);
  matrix.show(); //set all to off
  matrix.setPixelColor(56, 255, 255, 0);
  matrix.show();
  //set 7segment pins
  segmentDisplay.segmentPins(0, 1, 2, 3, 4, 5, 6, 7, HIGH);  // set high for common anode, and low for common cathode
  segmentDisplay.registerPins(serialPIN, registerClockPIN, serialClockPIN, 1);
  //start serial
  Serial.begin(115200);
  Serial.write('1');
}

void loop() { 
  command = "";
  value = "";
  int addTo = 0; //0 for command, 1 for value
  if(Serial.available()){
    while (Serial.available() > 0)
    {
      char readIn = (char)Serial.read();
      if (readIn == '\n') {
      	break;
      }
      else if (readIn == '|') {
      	addTo = 1;
      	continue;
      }
      //other stuff that is important
      if (addTo == 0) {
      	command += readIn;
      }
      else if (addTo == 1) {
      	value += readIn;
      }
    }
    response = interpretCommand(command,value);
    Serial.println(response); //sends response with \n at the end
  }
  //small delay
  delay(20);
}

String interpretCommand(String command, String value) {
	String responseString = "";

	if (command == "N") {
		// do 7 segment stuff
		segmentDisplay.write(value.toInt());
		responseString += value;
	}

	else {
		// do 8x8 stuff
		if (command == "T") {
			setToOT(value.toInt());
		}
		else if (command == "D") {
			setToDE(value.toInt());
		}
		else if (command == "E") {
			setToEM(value.toInt());
		}

	}

	return responseString;

}

void setToOT(int index) {
	matrix.setPixelColor(index, 255, 0, 0);
	matrix.show();

}

void setToDE(int index) {
	matrix.setPixelColor(index, 0, 255, 255);
	matrix.show();
}

void setToEM(int index) {
	matrix.setPixelColor(index, 0, 0, 0);
	matrix.show();
}
