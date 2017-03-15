#include "Arduino.h"
// https://www.dfrobot.com/wiki/index.php/12V_DC_Motor_251rpm_w/Encoder_(SKU:_FIT0186)

// The sample code for driving one way motor encoder
const byte rightEncoder0InterruptPinA = 2;  //A pin -> the interrupt pin 0
const byte rightEncoder0DigitalPinB = 4;  //B pin -> the digital pin 4
byte rightEncoder0IntPinALast;
int rightDuration;  //the number of the pulses
boolean rightDirection;  //the rotation direction

const byte leftEncoder0InterruptPinA = 3;  //A pin
const byte leftEncoder0DigitalPinB = 5;  //B pin
byte leftEncoder0IntPinALast;
int leftDuration;  //the number of the pulses
boolean leftDirection;  //the rotation direction

int leftInterruptCallCount = 0;
int rightInterruptCallCount = 0;

int leftIntReadDiffFromLastCount = 0;
int rightIntReadDiffFromLastCount = 0;


void setup()
{
  Serial.begin(115200);  // Initialize the serial port
  EncoderInit();  // Initialize the module
}

void loop()
{
  Serial.print("Pulse: right: ");
  Serial.print(rightDuration);
  Serial.print(" left: ");
  Serial.println(leftDuration);
    Serial.print("right call count: ");
    Serial.print(rightInterruptCallCount);
    Serial.print(" diff count: ");
    Serial.println(rightIntReadDiffFromLastCount);
    Serial.print("left call count: ");
    Serial.print(leftInterruptCallCount);
    Serial.print(" diff count: ");
    Serial.println(leftIntReadDiffFromLastCount);
  rightDuration = 0;
  leftDuration = 0;
  delay(100);
}

void EncoderInit()
{
  rightDirection = true;  //default -> Forward
  pinMode(rightEncoder0DigitalPinB,INPUT);
    attachInterrupt(digitalPinToInterrupt(rightEncoder0InterruptPinA), rightEncoderInterruptFunction, CHANGE);

    leftDirection = true;  //default -> Forward
    pinMode(leftEncoder0DigitalPinB,INPUT);
    attachInterrupt(digitalPinToInterrupt(leftEncoder0InterruptPinA), leftEncoderInterruptFunction, CHANGE);
}

/** original name "wheelSpeed" */
void rightEncoderInterruptFunction()
{
  ++rightInterruptCallCount;
  int Lstate = digitalRead(rightEncoder0InterruptPinA);
  if((rightEncoder0IntPinALast == LOW) && Lstate==HIGH)
  {
    ++rightIntReadDiffFromLastCount;
    int val = digitalRead(rightEncoder0DigitalPinB);
    if(val == LOW && rightDirection)
    {
      rightDirection = false; //Reverse
    }
    else if(val == HIGH && !rightDirection)
    {
      rightDirection = true;  //Forward
    }
  }
  rightEncoder0IntPinALast = Lstate;

  if(!rightDirection)  rightDuration++;
  else  rightDuration--;
}

void leftEncoderInterruptFunction()
{
    ++leftInterruptCallCount;
    int Lstate = digitalRead(leftEncoder0InterruptPinA);
    if((leftEncoder0IntPinALast == LOW) && Lstate==HIGH)
    {
        ++leftIntReadDiffFromLastCount;
        int val = digitalRead(leftEncoder0DigitalPinB);
        if(val == LOW && leftDirection)
        {
            leftDirection = false; //Reverse
        }
        else if(val == HIGH && !leftDirection)
        {
            leftDirection = true;  //Forward
        }
    }
    leftEncoder0IntPinALast = Lstate;

    if(!leftDirection)  leftDuration++;
    else  leftDuration--;
}
