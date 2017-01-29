#include <SoftwareSerial.h>

//import things here
#define PINA1 2
#define PINB1 4
#define PINA2 3
#define PINB2 5
byte PinA1Last;
byte PinA2Last;
int duration; //number of pulses
boolean Direction; //rotation direction

SoftwareSerial motorControl1(8,9); //RX,TX

String command = "";
String value = "";
String response = "";

void setup() {
 //start serial
  Serial.begin(115200);
  Serial.write('1');
  //setup pinmodes
  //pinMode(PINA1,INPUT);
  //pinMode(PINB1,INPUT);
  //pinMode(PINA2,INPUT);
  //pinMode(PINB2,INPUT);
  motorControl1.begin(115200);
  //set motor speed to zero
  motorControl1.write("1f0\r");
  delayMicroseconds(250);
  motorControl1.write("2f0\r");
  //initialize encoders
  //EncoderInit();
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
  duration = 0;
}

String interpretCommand(String command, String value) {
	//do motor things here
	String responseString = "n";
	if (command == "f") {
		String returnString = goForward();
		responseString = "1";
    responseString += returnString;
	}
	else if (command == "l") {
		String returnString = turnLeft();
		responseString = "1";
    responseString += returnString;
	}
	else if (command == "r") {
		String returnString = turnRight();
		responseString = "1";
    responseString += returnString;
	}
	return responseString;
}

void EncoderInit() {
  Direction = true; //default -> forward
  pinMode(PINB1,INPUT);
  attachInterrupt(0,wheelSpeed,CHANGE);
}

void wheelSpeed() {
  int Lstate = digitalRead(PINA1);
  if((PinA1Last == LOW) && Lstate == HIGH)
  {
    int val = digitalRead(PINB1);
    if (val == LOW && Direction)
    {
      Direction = false;
    }
    else if (val == HIGH && !Direction)
    {
      Direction = true;
    }
  }
  PinA1Last = Lstate;
  if (!Direction) duration++;
  else duration--;
}



int runMotorsTill(int value1, int value2, const char* comm1, const char* comm2) {
  unsigned long lastGoCommand = millis();
  int enc1 = 0;
  int enc2 = 0;
  bool on1 = true;
  bool on2 = true;
  //run motors
  motorControl1.write(comm1);
  delayMicroseconds(250);
  motorControl1.write(comm2);
  while (abs(enc1) < value1 || abs(enc2) < value2) {
    int state1 = digitalRead(PINA1);
    int state2 = digitalRead(PINA2);
    if((PinA1Last == LOW) && state1 == HIGH) {
      int val = digitalRead(PINB1);
      if (val == LOW) {
        enc1++;
      }
      else {
        enc1--;
      }
    }
    if((PinA2Last == LOW) && state2 == HIGH) {
      int val = digitalRead(PINB2);
      if (val == LOW) {
        enc2++;
      }
      else {
        enc2--;
      }
    }
    PinA1Last = state1;
    PinA2Last = state2;
    //Serial.println(enc1);
    //Serial.println(enc2);
    if (on1 && abs(enc1) >= value1) {
      motorControl1.write("1r0\r");
      on1 = false;
      delayMicroseconds(250);
    }
    if (on2 && abs(enc2) >= value2) {
      motorControl1.write("2r0\r");
      on2 = false;
      delayMicroseconds(250);
    }

    if (millis() - lastGoCommand > 500) {
      if (on1) {
        motorControl1.write(comm1);
        delayMicroseconds(250);
      }
      if (on2) {
        motorControl1.write(comm2);
        delayMicroseconds(250);
      }
      lastGoCommand = millis();
    }
  }
  motorControl1.write("1r0\r");
  delayMicroseconds(250);
  motorControl1.write("2r0\r");
  return enc1;
}

int checkEncodersTill(int value) {
  int enc1 = 0;
  int enc2 = 0;
  while (abs(enc1) < value) {
    int state1 = digitalRead(PINA1);
    if((PinA1Last == LOW) && state1 == HIGH) {
      int val = digitalRead(PINB1);
      if (val == LOW) {
        enc1++;
      }
      else {
        enc1--;
      }
    }
    PinA1Last = state1;
    Serial.println(enc1);
  }
  return enc1;
}

String goForward() {
  duration = runMotorsTill(1500,1500,"1f9\r","2f9\r");
  return String(duration);
}

String goForward_Int() {
  motorControl1.write("1f4\r");
  delayMicroseconds(250);
  motorControl1.write("2f4\r");
  //get encoder readings
  duration = 0;
  while (duration < 500)
  {
    //Serial.println(duration);
    delayMicroseconds(50);
  }
  //delay(500);
  motorControl1.write("1r0\r");
  delayMicroseconds(250);
  motorControl1.write("2r0\r");
  return String(duration);
}

void goForward_NoEnc() {
  motorControl1.write("1f2\r");
  delayMicroseconds(250);
  motorControl1.write("2f2\r");
  delay(500);
  motorControl1.write("1f0\r");
  delayMicroseconds(250);
  motorControl1.write("2f0\r");
}

String turnLeft() {
  duration = runMotorsTill(1500,500,"1f9\r","2r9\r");
  return String(duration);
}

String turnRight() {
  duration = runMotorsTill(500,1500,"1r9\r","2f9\r");
  return String(duration);
}