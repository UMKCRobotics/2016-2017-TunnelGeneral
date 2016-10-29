#include <SoftwareSerial.h>

//import things here


SoftwareSerial motorControl1(8,9);
SoftwareSerial motorControl2(10,11);

String command = "";
String value = "";
String response = "";

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
		responseString = "1";
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

void goForward() {
 motorControl1.write("1f9\r");
 motorControl2.write("1f9\r");
 delay(500);
 motorControl1.write("1f0\r");
 motorControl2.write("1f0\r");
}

void turnLeft() {
  //put code here
}

void turnRight() {
  //put code here
}