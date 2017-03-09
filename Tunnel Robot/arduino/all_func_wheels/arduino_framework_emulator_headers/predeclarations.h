#ifndef PREDECLARATIONS_H_INCLUDED
#define PREDECLARATIONS_H_INCLUDED

void EncoderInit();
void MotorInit();
void ButtonInit();

String interpretCommand(String command, String value);

String goForward();
String turnLeft();
String turnRight();

String calibrateWithIR(String side);
String performTap();


#endif // PREDECLARATIONS_H_INCLUDED
