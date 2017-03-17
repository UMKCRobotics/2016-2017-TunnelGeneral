#include "MotorControllerCopyDontEdit.h"

#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

/** movement motor pins are defined in MotorController.h */

//parsing inputs
String command; //used to store command from serial
String value; //used to store value from serial
String response; //used to store response to main program

void leftEncoderInterruptFunction();
void rightEncoderInterruptFunction();

MotorInterface motorInterface(leftEncoderInterruptFunction, rightEncoderInterruptFunction);
MotorController motorController(&motorInterface);

volatile long lastLeftInterrupt;
volatile long lastRightInterrupt;


void setup() {
  //start serial
  Serial.begin(115200);
  //start serial1 to motor controller
  //Serial1.begin(115200);
  //initialize encoders/motors
  motorInterface.encoderInit();
  motorInterface.motorInit();
  //send READY byte
  Serial.write('1');

  long startTime = millis();

  motorInterface.setMotorPower(LEFT, 150, 1);
  motorInterface.setMotorPower(RIGHT, 150, 1);

  delay(2000);

  long timeBeforeStop = millis();

  motorInterface.setMotorPower(LEFT, 0, 1);
    motorInterface.setMotorPower(RIGHT, 0, 1);
}

void loop()
{
    long lastPrintedLeft;
    long lastPrintedRight;

    if (lastLeftInterrupt != lastPrintedLeft || lastRightInterrupt != lastPrintedRight)
    {
        Serial.print(lastLeftInterrupt);
        Serial.print(' ');
        Serial.println(lastRightInterrupt);

        lastPrintedLeft = lastLeftInterrupt;
        lastPrintedRight = lastRightInterrupt;
    }
}

void loopOld() {

  command = "";
  value = "";
  String* addTo = &command;  // which information we are reading from serial
  //ButtonStates();
  if(Serial.available()){
    while (Serial.available() > 0)
    {
      char readIn = (char)Serial.read();
      if (readIn == '\n') {
        break;
      }
      else if (readIn == '|') {
        addTo = &value;  // value comes after the '|'
        continue;
      }
      //other stuff that is important
      (*addTo) += readIn;
    }
    //clear anything remaining in serial
    while (Serial.available() > 0) {
      Serial.read();
    }
    response = interpretCommand(command,value);
    Serial.println(response); //sends response with \n at the end
  }
  //small delay
  delay(20);

}


String interpretCommand(String command, String value) {
  
  String responseString = "n";  // what this function returns
  String returnString = "";     // holds the return value of the command function
  const String responseHeader = "\t1";

  // commands to read odometers
  if (command == "<") {
    returnString = "";
    responseString = responseHeader;
    responseString += motorInterface.readEncoder(LEFT);
  }
  else if (command == ">") {
    returnString = "";
    responseString = responseHeader;
    responseString += motorInterface.readEncoder(RIGHT);
  }

  //check if motor stuff
  else if (command == "f") {
    motorController.go(FORWARD);
    returnString = "1";
    responseString = responseHeader;
    responseString += returnString;
  }
  else if (command == "l") {
    motorController.go(LEFT);
    returnString = "1";
    responseString = responseHeader;
    responseString += returnString;
  }
  else if (command == "r") {
    motorController.go(RIGHT);
    returnString = "1";
    responseString = responseHeader;
    responseString += returnString;
  }

  //check if any BADs were obtained
  if (returnString == "BAD") {
    return "n";
  }
  return responseString;

}

//START OF MOTOR STUFF

void rightEncoderInterruptFunction() {
  motorInterface.encoderInterrupt(RIGHT);
  lastRightInterrupt = millis();
}

void leftEncoderInterruptFunction() {
  motorInterface.encoderInterrupt(LEFT);
  lastLeftInterrupt = millis();
}

//ACTUAL implementation for direct motor controller
int runMotorsTill(int value1, int value2, int pwm1, int pwm2) {
  //Serial.print("value1 = ");
  //Serial.print(value1);
  //Serial.print("value2 = ");
  //Serial.print(value2);
  //used for running time calculations
  unsigned long goT1 = 0;
  unsigned long goT2 = 0;
  //amount of running time
  int duration1 = 0;
  int duration2 = 0;
  //used to tell if motor should still be running
  bool on1 = true;
  bool on2 = true;
  
  //run motors
  //set direction for motor 1
  motorInterface.changeDirection(RIGHT, pwm1>=0);
  motorInterface.changeDirection(LEFT, pwm2>=0);
  
  //if either motor should be running
  while (on1 || on2) {
    //if motor1 should be running
    if (on1) {
      //stop motor1
      if (duration1 >= value1) {
        analogWrite(RIGHT_MOTOR_PWM, 0);
        digitalWrite(LED1,LOW);
        on1 = false;
      }
      //if motor should be running
      else if (duration1 <= value1) {
        //Serial.println(rightEncoderOdometer);
        analogWrite(RIGHT_MOTOR_PWM, 150);
        //updates durration if need be
        //changes initial start condition
        if(goT1 == 0)
          goT1 = millis();
        else
          duration1 += (int) millis() - goT1;
          goT1 = millis();
      }
    }
    //if motor2 should be running
    if (on2) {
      //stop motor2
      if (duration2 >= value2) {
        analogWrite(LEFT_MOTOR_PWM, 0);
        digitalWrite(LED2,LOW);
        on2 = false;
      }
      //if motor should be running
      else if (duration2 <= value2) {
        //Serial.println(leftEncoderOdometer);
        analogWrite(LEFT_MOTOR_PWM, 150);
        //updates durration if need be
        //changes initial start condition
        if(goT2 == 0)
          goT2 = millis();
        else
            duration2 += (int) millis() - goT2;
          goT2 = millis();
      }
    }
  }
  //stop both motors now, promptly
  analogWrite(RIGHT_MOTOR_PWM, 0);
  analogWrite(LEFT_MOTOR_PWM, 0);

  return 1;
}
