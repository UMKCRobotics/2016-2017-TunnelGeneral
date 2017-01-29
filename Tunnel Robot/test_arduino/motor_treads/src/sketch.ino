#include <SoftwareSerial.h>

//import things here
#define PINA1 2
#define PINB1 4
#define PINA2 3
#define PINB2 5

SoftwareSerial motorControl1(8,9); //RX,TX
SoftwareSerial motorControl2(10,11);

String command = "";
String value = "";
String response = "";


int QEM[16] = {0,-1,1,2,1,0,2,-1,-1,2,0,1,2,1,-1,0};

void setup() {
 //start serial
  Serial.begin(115200);
  Serial.write('1');
  motorControl1.begin(115200);
  motorControl2.begin(115200);

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
	//do motor things here
	String responseString = "n";
	if (command == "f") {
		goForward();
		responseString = "1good";
	}
	else if (command == "l") {
		turnLeft();
		responseString = "1";
	}
	else if (command == "r") {
		turnLeft();
		responseString = "1";
	}
	return responseString;
}

void ISR0(long value) {
  volatile unsigned char Old0, New0;
  volatile long Position0 = 0;
  while (abs(Position0) < value) {
    Old0 = New0;
    New0 = digitalRead(PINA2)*2 + digitalRead(PINB2); //convert binary to decimal
    Position0 += QEM[Old0*4 + New0];
    Serial.println(Position0);
  }
}

void goForward() {
 motorControl1.write("1r9\r");
 delayMicroseconds(250);
 motorControl1.write("2r9\r");
 delayMicroseconds(250);
 motorControl2.write("1r9\r");
 delayMicroseconds(250);
 motorControl2.write("2r9\r");
 //check encoders here
 ISR0(500);
 //delay(500);
 motorControl1.write("1f0\r");
 delayMicroseconds(250);
 motorControl1.write("2f0\r");
 delayMicroseconds(250);
 motorControl2.write("1f0\r");
 delayMicroseconds(250);
 motorControl2.write("2f0\r");
}

void turnLeft() {
  //put code here
}

void turnRight() {
  //put code here
}