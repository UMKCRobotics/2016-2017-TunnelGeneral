#include "PID.h"
#include "RedBot.h"
#define FRONT_IR A0
#define BACK_IR A2

RedBotMotors motors;
PID<int> pid;

void setup()
{
	Serial.begin(9600);
}

void loop()
{
	int tolerance = 40;
	int frontReading = analogRead(FRONT_IR);
	int backReading = analogRead(BACK_IR);
	while(calculateDifference(frontReading,backReading) > tolerance){
		frontReading = analogRead(FRONT_IR);
		backReading = analogRead(BACK_IR);
		motors.pivot(-pid.calculate(backReading - frontReading, 0),10);
		pid.logDiagnosticInfo();
		delay(1000);
	}
	motors.stop();
}

int calculateDifference(int frontReading, int backReading){
	return abs(backReading - frontReading);
}

int calculateSpeed(int difference, int tolerance){
	if(difference > tolerance * 2){
		return 100;
	}
	return 100 / (difference / tolerance);
}
