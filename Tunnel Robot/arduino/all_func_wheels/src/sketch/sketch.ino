#include "Adafruit_GFX.h"
#include "Adafruit_NeoMatrix.h"
#include "Adafruit_NeoPixel.h"

#include "PID.h"

#include "MotorController.h"

#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif
//test LED pins
#define LED1 22
#define LED2 23
//EMF PINS
#define EMF1 A0
//IR PINS
#define IR_L1 A14
#define IR_L2 A13
#define IR_B1 A11
#define IR_B2 A12
#define IR_R1 A9
#define IR_R2 A10
#define IR_F1 A8

//8x8 pin
#define PINM 6
//7 segment pins
#define DATA 7     // serial pin
#define LATCH 9   // register clock pin
#define CLK 8     // serial clock pin
//tapper pins
#define TAPPER_ENCODER_INTERRUPT_PIN 20
#define TAPPER_ENCODER_DIGITAL_PIN 26
volatile byte tapperEncoderIntPinLast;
volatile int tapperEncoderOdometer;
volatile boolean tapperEncoderDirection;
//tapper control
#define MOT3_PIN1 50
#define MOT3_PIN2 48
#define MOT3_PWM 46

/** movement motor pins are defined in MotorController.h */

//motor modulus
int motorMod = 0;

// just so we don't need to delete all the switch code
#define SWITCH_FR 1
#define SWITCH_FL 1

//button pins
#define GoPin 18 //Go button - INTERRUPT PIN
#define StopPin 19 //Stop button - INTERRUPT PIN
//button states
volatile char GoState = '0';
volatile char StopState = '0';  // should these be initialized in the setup function instead of here?
//sensor thresholds
int EMF_thresh = 45;
//digit representations
int digits[10] = {190,6,218,206,102,236,252,134,254,238};
//matrix setup
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, PINM,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB           + NEO_KHZ800);
//parsing inputs
String command; //used to store command from serial
String value; //used to store value from serial
String response; //used to store response to main program

void leftEncoderInterruptFunction();
void rightEncoderInterruptFunction();

MotorInterface motorInterface(leftEncoderInterruptFunction, rightEncoderInterruptFunction);
MotorController motorController(&motorInterface);

void ButtonStates(){
  int button1 = digitalRead(GoPin);
  int button2 = digitalRead(StopPin);
  if (button1 == HIGH) {
    GoState = '1';
  }
  if (button2 == HIGH)
    StopState = '0';
}

void GoButtonFunc() {
  int buttonStateGo = digitalRead(GoPin);
  if (buttonStateGo == HIGH) {
    GoState = '1';
  }
}
void StopButtonFunc() {
  int buttonStateStop = digitalRead(StopPin);
  if (buttonStateStop == HIGH) {
    StopState = '0';
  }
}

void setup() {
  //initialize led pins
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  //initialize buttons
  pinMode(GoPin, INPUT); //setting pins for green button
  pinMode(StopPin, INPUT); //setting pins for red button
  //initialize matrix
  matrix.begin();
  matrix.setBrightness(20);
  matrix.show(); //set all to off
  //setup 7 digit display and clear it
  pinMode(LATCH, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  clearDigit();
  //start serial
  Serial.begin(115200);
  //start serial1 to motor controller
  //Serial1.begin(115200);
  //initialize encoders/motors
  motorInterface.encoderInit();
  motorInterface.motorInit();
  tapperEncoderInit();
  tapperMotorInit();
  ButtonInit();
  //SwitchInit();
  //Serial1.write("1f0\r");
  //delayMicroseconds(500);
  //Serial1.write("2f0\r");
  //send READY byte
  Serial.write('1');

}

void loop() {

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
    motorController.goForward();
    returnString = "1";
    responseString = responseHeader;
    responseString += returnString;
  }
  else if (command == "l") {
    motorController.turnLeft();
    returnString = "1";
    responseString = responseHeader;
    responseString += returnString;
  }
  else if (command == "r") {
    motorController.turnRight();
    returnString = "1";
    responseString = responseHeader;
    responseString += returnString;
  }
  else if (command == "c") {
    if (value == "")
      returnString = "0";  // returnString = calibrateWithSwitches();
    else
      returnString = calibrateWithIR(value);
    responseString = responseHeader;
    responseString += returnString;
  }
  else if (command == "A") {
    //perform tap
    returnString = performTap();
    responseString = responseHeader;
    responseString += returnString;
  }

  //check if 7 segment stuff
  else if (command == "N") {
    // do 7 segment stuff
    displayDigit(value.toInt());
    responseString = responseHeader;
    responseString += value;
  }

  //check if button stuff
  else if (command == "B") {
    if (value == "G") {
      responseString = responseHeader;
      responseString += GoState;
    }
    else if (value == "S") {
      responseString = responseHeader;
      responseString += StopState;
    }
  }
  //check if 8x8 stuff
  else if (command == "R") {
    setReadyLight();
    responseString = responseHeader;
  }
  else if (command == "T") {
    setToOT(value.toInt());
    responseString = responseHeader;
  }
  else if (command == "D") {
    setToDE(value.toInt());
    responseString = responseHeader;
  }
  else if (command == "E") {
    setToEM(value.toInt());
    responseString = responseHeader;
  }
  //check if sensor stuff
  else if (command == "e") {
    responseString = responseHeader;
    responseString += String(readEMF());
  }
  else if (command == "S") {
    if (value == "E") {
      responseString = responseHeader;
      responseString += String(readEMF());
    }
    else if (value == "O") {
      responseString = responseHeader;
      responseString += getObstacleReport();
    }
    else if (value == "F") {
      responseString = responseHeader;
      //lol good luck with that
    }
  }

  //check if any BADs were obtained
  if (returnString == "BAD") {
    return "n";
  }
  return responseString;

}

//START OF MOTOR STUFF
void tapperEncoderInit() {
  pinMode(TAPPER_ENCODER_DIGITAL_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(TAPPER_ENCODER_INTERRUPT_PIN), tapperEncoderInterruptFunction, CHANGE);
}

void tapperMotorInit() {
  pinMode(MOT3_PIN1,OUTPUT);
  pinMode(MOT3_PIN2,OUTPUT);
  pinMode(MOT3_PWM,OUTPUT);
}

void SwitchInit() {
  pinMode(SWITCH_FR,INPUT);
  pinMode(SWITCH_FL,INPUT);
}

void rightEncoderInterruptFunction() {
  motorInterface.encoderInterrupt(RIGHT);
}

void leftEncoderInterruptFunction() {
  motorInterface.encoderInterrupt(LEFT);
}

void tapperEncoderInterruptFunction() {
  int Lstate = digitalRead(TAPPER_ENCODER_INTERRUPT_PIN);
  if((tapperEncoderIntPinLast == LOW) && Lstate == HIGH)
  {
    int val = digitalRead(TAPPER_ENCODER_DIGITAL_PIN);
    if (val == LOW && tapperEncoderDirection)
    {
      tapperEncoderDirection = false;
    }
    else if (val == HIGH && !tapperEncoderDirection)
    {
      tapperEncoderDirection = true;
    }
  }
  tapperEncoderIntPinLast = Lstate;
  if (!tapperEncoderDirection) tapperEncoderOdometer++;
  else tapperEncoderOdometer--;
}

void setTapperDirection(int pwm3) {
  if (pwm3 >= 0) {
    digitalWrite(MOT3_PIN1,HIGH);
    digitalWrite(MOT3_PIN2,LOW);
  }
  else {
    digitalWrite(MOT3_PIN1,LOW);
    digitalWrite(MOT3_PIN2,HIGH);
  }
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

//ACTUAL implementation for tapping
int runTappingTill(int value3, int pwm3) {
  tapperEncoderOdometer = 0;
  bool on3 = true;
  int slowDiff = 400;
  int slowPWM = 255;
  int slowestPWM = 240;
  //set motor direction
  setTapperDirection(pwm3);
  //set PWM for motor to start it up
  analogWrite(MOT3_PWM,abs(pwm3));
  //do stuff while not done
  while (on3) {
    //if StopButton has been pressed, stop moving!
    if (StopState == '1') {
      break;
    }
    if (on3) {
      if (abs(tapperEncoderOdometer) >= value3) {
        analogWrite(MOT3_PWM,0);
        on3 = false;
      }
      else if (abs(tapperEncoderOdometer) >= value3-slowDiff) {
        int actualPWM3 = map(abs(tapperEncoderOdometer),value3-slowDiff,value3,slowPWM,slowestPWM);
        analogWrite(MOT3_PWM,actualPWM3-5);
      }
    }
  }
  //stop motor now!
  analogWrite(MOT3_PWM,0);

  return 1;
}

int expMovingAvg(int newVal,int oldVal, double prefVal) {
  return int(newVal*prefVal + oldVal*(1.0-prefVal));
}

int runCalibrationPivotIR(int pin1, int pin2, int setPoint, int tolerance) {
  //1 is on the right, 2 is on the left, let's just roll with it, okay?
  int reading1 = analogRead(pin1);
  int reading2 = analogRead(pin2);
  int lastReading1 = reading1;
  int lastReading2 = reading2;
  double readingPref = 0.6;
  int readingDiff = reading1-reading2;
  int pidVal;
  int correctedPidVal;
  int minPWM = 65;
  int maxPWM = 200;
  int actualPWM;
  //start PID, set proportional/integral/derivative gains 
  PID<int> pid(0.6,0.001,0);
  //do movement while readings are outside tolerance range
  int countGood = 0;
  int countRequired = 6;
  while (countGood < countRequired) {
    //first check if stop button has been pressed
    if (StopState == '1') {
      digitalWrite(LED1,LOW);
      digitalWrite(LED2,LOW);
      break;
    }
    if (readingDiff < setPoint+tolerance && readingDiff > setPoint-tolerance) {
      countGood += 1;
      analogWrite(RIGHT_MOTOR_PWM,0);
      analogWrite(LEFT_MOTOR_PWM,0);
    }
    else {
      countGood = 0;
      //calculate what PWM to use
      pidVal = pid.calculate(readingDiff, setPoint);
      //change equation for actualPWM to be reasonable with pidVals
      actualPWM = min(minPWM + abs(pidVal),maxPWM);
      //change directions appropriately, and use actualPWM
      motorInterface.changeDirection(RIGHT, pidVal >= 0);
      motorInterface.changeDirection(LEFT, (0-pidVal) >= 0);
      analogWrite(RIGHT_MOTOR_PWM,actualPWM);
      analogWrite(LEFT_MOTOR_PWM,actualPWM);
    }
    //get new measurements now
    reading1 = expMovingAvg(analogRead(pin1),lastReading1,readingPref);
    reading2 = expMovingAvg(analogRead(pin2),lastReading2,readingPref);
    lastReading1 = reading1;
    lastReading2 = reading2;
    readingDiff = reading1-reading2;
    //now wait a little bit before trying again
    //pid.logDiagnosticInfo();
    //Serial.println(readingDiff);
    delay(10);
  }
  //stop both motors
  analogWrite(RIGHT_MOTOR_PWM,0);
  analogWrite(LEFT_MOTOR_PWM,0);

  return 1;
}



String goForwardOld() {
  //int actualDur = runMotorsTill(1500,1500,"1f9\r","2f9\r");
  int forwCount = 2512;
  int actualDur = runMotorsTill(forwCount,forwCount+20,251,255);
  return "1";
}

String performTap() {
  int encCount = 1725;
  int actualDur = runTappingTill(encCount,255);
  return "1";
}

String calibrateWithIR(String side) {
  //if L, use IR on left side
  int threshold = 5;
  if (side == "L")  
    runCalibrationPivotIR(IR_L1,IR_L2,57,threshold);
  //if R, use IR on right side
  else if (side == "R")  
    runCalibrationPivotIR(IR_R1,IR_R2,30,threshold);
  //if B, use IR on back side
  else if (side == "B")  
    runCalibrationPivotIR(IR_B1,IR_B2,0,threshold);
  /*
  //if F, use IR on front side (might not use)
  else if (side == "F")  
    runCalibrationPivotIR(IR_F1,IR_F2,0,threshold);
  */
  //signal if bad side received
  else
    return "BAD";
  return "1";
}

String goBackward() {
  //int actualDur = runMotorsTill(1500,1500,"1f9\r","2f9\r");
  int backCount = 2512;
  int actualDur = runMotorsTill(backCount,backCount,-255,-255);
  return "1";
}

//actually turns right
String turnLeft() {
  //int actualDur = runMotorsTill(1050,1050,"1f9\r","2r9\r");
  int actualDur = runMotorsTill(1500,1500,255,-255);
  return "1";
}

//actually turns left
String turnRight() {
  //int actualDur = runMotorsTill(1100,1100,"1r9\r","2f9\r");
  int actualDur = runMotorsTill(1500,1500,-255,255);
  return "1";
}
//END OF MOTOR STUFF

//START OF SENSOR STUFF

int getIRReading(int whichPin) {
  // average of lots of readings
  const long sampleCount = 200;
  long total = 0;
  for (int i = sampleCount; i > 0; --i) {
    total += analogRead(whichPin);
  }

  return total / sampleCount;
}

String getObstacleReport() {
  String report = "";
  //report format: right,front,left,back
  const int threshold = 190;  // set this to something reasonable
  //check right
  Serial.print("IR_R1 giving ");
  Serial.print(getIRReading(IR_R1));
  if (getIRReading(IR_R1) > threshold || getIRReading(IR_R2) > threshold)
    report += '1';
  else
    report += '0';
  //check front
  if (getIRReading(IR_F1) > threshold)  // || getIRReading(IR_F2) > threshold)
    report += '1';
  else
    report += '0';
  //check left
  if (getIRReading(IR_L1) > threshold || getIRReading(IR_L2) > threshold)
    report += '1';
  else
    report += '0';
  //check back
  if (getIRReading(IR_B1) > threshold || getIRReading(IR_B2) > threshold)
    report += '1';
  else
    report += '0';

  //return the report
  return report;
}

int readEMF() {
  return getEMFreading(EMF1);
}

int getEMFreading(int port) {
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


//START OF DISPLAY STUFF
void ButtonInit() {
  pinMode(GoPin,INPUT);
  pinMode(StopPin,INPUT);
  attachInterrupt(digitalPinToInterrupt(GoPin),GoButtonFunc, CHANGE);
  attachInterrupt(digitalPinToInterrupt(StopPin),StopButtonFunc, CHANGE);
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

void setReadyLight() {
  matrix.setPixelColor(56, 255, 255, 0);
  matrix.show();
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
//END OF DISPLAY STUFF
