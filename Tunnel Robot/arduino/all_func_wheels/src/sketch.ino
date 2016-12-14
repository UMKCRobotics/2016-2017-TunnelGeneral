#include "Adafruit_GFX.h"
#include "Adafruit_NeoMatrix.h"
#include "Adafruit_NeoPixel.h"
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif
//test LED pins
#define LED1 22
#define LED2 23


//8x8 pin
#define PINM 6
//7 segment pins
#define DATA 7     // serial pin
#define LATCH 9   // register clock pin
#define CLK 8     // serial clock pin
//encoder pins (PINAx MUST be interrupts)
//NOTE: MOTOR1 is RIGHT, MOTOR2 is LEFT
#define PINA1 2
#define PINB1 4
#define PINA2 3
#define PINB2 5
volatile byte PinA1Last;
volatile byte PinA2Last;
volatile int duration1;
volatile int duration2;
volatile boolean Direction1;
volatile boolean Direction2;
//motor control
#define MOT1_PIN1 40
#define MOT1_PIN2 42
#define MOT1_PWM 44
#define MOT2_PIN1 41
#define MOT2_PIN2 43
#define MOT2_PWM 45
//motor modulus
int motorMod = 0;
//button pins
const int GoPin = 10; //Go button
const int StopPin = 11; //Stop button
//button states
char GoState = '0';
char StopState = '0';
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

void ButtonStates(){
  int button1 = digitalRead(GoPin);
  int button2 = digitalRead(StopPin);
  if (button1 == HIGH) {
    GoState = '1';
  }
  if (button2 == HIGH)
    StopState = '1';
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
  matrix.setPixelColor(56, 255, 255, 0);
  matrix.show();
  //setup 7 digit display and clear it
  pinMode(LATCH, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  clearDigit();
  //start serial
  Serial.begin(115200);
  //start serial1 to motor controller
  Serial1.begin(115200);
  //initialize encoders/motors
  EncoderInit();
  MotorInit();
  //Serial1.write("1f0\r");
  //delayMicroseconds(500);
  //Serial1.write("2f0\r");
  //send READY byte
  Serial.write('1');
}

void loop() { 
  command = "";
  value = "";
  int addTo = 0; //0 for command, 1 for value
  ButtonStates();
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
  
  String responseString = "n";

  //check if motor stuff
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

  //check if 7 segment stuff
  else if (command == "N") {
    // do 7 segment stuff
    displayDigit(value.toInt());
    responseString = "1";
    responseString += value;
  }

  //check if button stuff
  else if (command == "B") {
    if (value == "G") {
      responseString = "1";
      responseString += GoState;
    }
    else if (value == "S") {
      responseString = "1";
      responseString += StopState;
    }
  }

  //check if 8x8 stuff
  else {
    // do 8x8 stuff
    if (command == "T") {
      setToOT(value.toInt());
      responseString = "1";
    }
    else if (command == "D") {
      setToDE(value.toInt());
      responseString = "1";
    }
    else if (command == "E") {
      setToEM(value.toInt());
      responseString = "1";
    }
  }

  return responseString;

}

//START OF MOTOR STUFF
void EncoderInit() {
  pinMode(PINB1,INPUT);
  pinMode(PINB2,INPUT);
  attachInterrupt(digitalPinToInterrupt(PINA1),wheelSpeed1,CHANGE);
  attachInterrupt(digitalPinToInterrupt(PINA2),wheelSpeed2,CHANGE);
}

void MotorInit() {
  pinMode(MOT1_PIN1,OUTPUT);
  pinMode(MOT1_PIN2,OUTPUT);
  pinMode(MOT1_PWM,OUTPUT);
  pinMode(MOT2_PIN1,OUTPUT);
  pinMode(MOT2_PIN2,OUTPUT);
  pinMode(MOT2_PWM,OUTPUT);
}

void wheelSpeed1() {
  int Lstate = digitalRead(PINA1);
  if((PinA1Last == LOW) && Lstate == HIGH)
  {
    int val = digitalRead(PINB1);
    if (val == LOW && Direction1)
    {
      Direction1 = false;
    }
    else if (val == HIGH && !Direction1)
    {
      Direction1 = true;
    }
  }
  PinA1Last = Lstate;
  if (!Direction1) duration1++;
  else duration1--;
}

void wheelSpeed2() {
  int Lstate = digitalRead(PINA2);
  if((PinA2Last == LOW) && Lstate == HIGH)
  {
    int val = digitalRead(PINB2);
    if (val == LOW && Direction2)
    {
      Direction2 = false;
    }
    else if (val == HIGH && !Direction2)
    {
      Direction2 = true;
    }
  }
  PinA2Last = Lstate;
  if (!Direction2) duration2++;
  else duration2--;
}

int runMotorsTill(int value1, int value2, const char* comm1, const char* comm2) {
  unsigned long lastGoCommand = millis();
  duration1 = 0;
  duration2 = 0;
  bool on1 = true;
  bool on2 = true;
  //run motors
  Serial1.write(comm1);
  digitalWrite(LED1,HIGH);
  delayMicroseconds(500);
  Serial1.write(comm2);
  digitalWrite(LED2,HIGH);
  delayMicroseconds(500);
  //do stuff while not done
  while (on1 || on2) {
    if (on1 && abs(duration1) >= value1) {
      Serial1.write("1f0\r");
      digitalWrite(LED1,LOW);
      on1 = false;
      delayMicroseconds(500);
    }
    if (on2 && abs(duration2) >= value2) {
      Serial1.write("2f0\r");
      digitalWrite(LED2,LOW);
      on2 = false;
      delayMicroseconds(500);
    }
  }
  //stop both motors now, promptly
  Serial1.write("1r0\r");
  delayMicroseconds(500);
  Serial1.write("2r0\r");

  return 1;
}

void changeDirection(int pwm1, int pwm2) {
  if (pwm1 >= 0) {
    digitalWrite(MOT1_PIN1,HIGH);
    digitalWrite(MOT1_PIN2,LOW);
  }
  else {
    digitalWrite(MOT1_PIN1,LOW);
    digitalWrite(MOT1_PIN2,HIGH);
  }
  //set direction for motor 2
  if (pwm2 >= 0) {
    digitalWrite(MOT2_PIN1,HIGH);
    digitalWrite(MOT2_PIN2,LOW);
  }
  else {
    digitalWrite(MOT2_PIN1,LOW);
    digitalWrite(MOT2_PIN2,HIGH);
  }
}

int runMotorsTill(int value1, int value2, int pwm1, int pwm2) {
  unsigned long lastGoCommand = millis();
  duration1 = 0;
  duration2 = 0;
  bool on1 = true;
  bool on2 = true;
  //run motors
  //set direction for motor 1
  changeDirection(pwm1,pwm2);
  //set PWM for both with as little latency in between
  analogWrite(MOT1_PWM,abs(pwm1));
  digitalWrite(LED1,HIGH);
  analogWrite(MOT2_PWM,abs(pwm2));
  digitalWrite(LED2,HIGH);
  //do stuff while not done
  while (on1 || on2) {
    if (on1 && abs(duration1) >= value1) {
      analogWrite(MOT1_PWM,0);
      digitalWrite(LED1,LOW);
      on1 = false;
    }
    if (on2 && abs(duration2) >= value2) {
      analogWrite(MOT2_PWM,0);
      digitalWrite(LED2,LOW);
      on2 = false;
    }
  }
  //stop both motors now, promptly
  analogWrite(MOT1_PWM,0);
  analogWrite(MOT2_PWM,0);

  return 1;
}

String goForward() {
  //int actualDur = runMotorsTill(1500,1500,"1f9\r","2f9\r");
  int forwCount = 2300;
  int actualDur = runMotorsTill(forwCount-50,forwCount,255,255);
  return "1";
}

String goBackward() {
  //int actualDur = runMotorsTill(1500,1500,"1f9\r","2f9\r");
  int actualDur = runMotorsTill(1500,1500,-255,-255);
  return "1";
}

String turnLeft() {
  //int actualDur = runMotorsTill(1050,1050,"1f9\r","2r9\r");
  int actualDur = runMotorsTill(1050,1050,255,-255);
  return "1";
}

String turnRight() {
  //int actualDur = runMotorsTill(1100,1100,"1r9\r","2f9\r");
  int actualDur = runMotorsTill(1100,1100,-255,255);
  return "1";
}
//END OF MOTOR STUFF


//START OF DISPLAY STUFF
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
//END OF DISPLAY STUFF
