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

void displayDigit(int dig);

void setReadyLight();
void setToOT(int index);
void setToDE(int index);
void setToEM(int index);

int readEMF();
String getObstacleReport();

void wheelSpeed1();
void wheelSpeed2();
void wheelSpeed3();

#endif // PREDECLARATIONS_H_INCLUDED
