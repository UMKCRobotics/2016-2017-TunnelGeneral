#include "Adafruit_GFX.h"
#include "Adafruit_NeoMatrix.h"
#include "Adafruit_NeoPixel.h"
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

#define PIN 6

#define DATA 2					 // serial pin
#define LATCH 4		// register clock pin
#define CLK 3			// serial clock pin


int digits[10] = {190,6,218,206,102,236,252,134,254,238};

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, PIN,
	NEO_MATRIX_TOP		 + NEO_MATRIX_RIGHT +
	NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
	NEO_GRB						+ NEO_KHZ800);

String command; //used to store command from serial
String value; //used to store value from serial
String response; //used to store response to main program




void setup() {
	matrix.begin();
	matrix.setBrightness(20);
	matrix.show(); //set all to off
	matrix.setPixelColor(56, 255, 255, 0);
	matrix.show();
	//setup 7 digit display and clear it
	pinMode(LATCH, OUTPUT);
	pinMode(CLK, OUTPUT);
	pinMode(DATA, OUTPUT);
	clearDigit();
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
		displayDigit(value.toInt());
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

void displayDigit(int dig) {
	for (int i = digits[dig]; i <= digits[dig]+1; i++) {
		digitalWrite(LATCH, HIGH);
		int number = i;
		shiftOut(DATA, CLK, MSBFIRST, ~(char)number); // digitOne
		digitalWrite(LATCH, LOW);
		delay(1);
	}
}

void clearDigit() {
	digitalWrite(LATCH, HIGH);
	int number = 0;
	shiftOut(DATA, CLK, MSBFIRST, ~(char)number); // digitOne
	digitalWrite(LATCH, LOW);
	delay(1);
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
