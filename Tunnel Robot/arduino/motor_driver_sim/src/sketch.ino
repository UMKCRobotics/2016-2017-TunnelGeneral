//import things here
#include <SoftwareSerial.h>

SoftwareSerial motorControl1(8,9); //RX,TX


//String command = "";
//String value = "";
String response = "";

void setup() {
 //start serial
  pinMode(13,OUTPUT);
  Serial.begin(115200);
  Serial.write('1');
  motorControl1.begin(115200);

}

void loop() { 
  String command = "";
  String value = "";
  int addTo = 0; //0 for command, 1 for value
  if(motorControl1.available()){
    while (motorControl1.available() > 0)
    {
      char readIn = (char)motorControl1.read();
      if (readIn == '\r') {
      	break;
      }
      command += readIn;
      
    }
    if (command.length() == 3) {
      response = interpretCommand(command,value);
    }
    Serial.println(command.length());
    Serial.println(command);
    //Serial.println(response); //sends response with \n at the end
  }
  //small delay
  delay(20);
}

String interpretCommand(String command, String value) {
	//do motor things here
  if(command.length() == 3 && (command[0] == '1' || command[0] == '2') && (command[1]=='f' || command[1]=='r') && (isdigit(command[2]))) {
    digitalWrite(13,HIGH);
    delay(1000);
    digitalWrite(13,LOW);
  }
  else {
    digitalWrite(13,HIGH);
    delay(250);
    digitalWrite(13,LOW);
    delay(250);
    digitalWrite(13,HIGH);
    delay(250);
    digitalWrite(13,LOW);
  }

	return "1";
}