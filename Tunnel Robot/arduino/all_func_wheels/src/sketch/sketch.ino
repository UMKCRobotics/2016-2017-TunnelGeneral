#include "Adafruit_GFX.h"
#include "Adafruit_NeoMatrix.h"
#include "Adafruit_NeoPixel.h"

// lots of debug messages to serial
// TODO: remove this, but only if we've done enough testing without it
#define VERBOSE

#include "PID.h"

// #include "MovementInterface.h"  // old cartesian strategy
#include "MovementInterfaceMasterSlave.h"
#include "Calibrator.h"
#include "Buttons.h"
#include "ObstacleFinder.h"

#ifndef PSTR
#define PSTR // Make Arduino Due happy
#endif


//test LED pins
#define LED1 22
#define LED2 23
//EMF PINS
#define EMF1 A0

/** IR pins are defined in IRPins.h */

/** movement motor pins are defined in MotorInterface.h */

/** button pins are defined in Buttons.h */

//8x8 pin
#define PINM 6
#define MATRIX_READY_LIGHT_PIXEL 56
//matrix setup
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, PINM,
                                               NEO_MATRIX_TOP + NEO_MATRIX_RIGHT +
                                               NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
                                               NEO_GRB + NEO_KHZ800);

//7 segment pins
#define DATA 7     // serial pin
#define LATCH 9   // register clock pin
#define CLK 8     // serial clock pin
//digit representations
int digits[10] = {190, 6, 218, 206, 102, 236, 252, 134, 254, 238};

//parsing inputs
String command; //used to store command from serial
String value; //used to store value from serial
String response; //used to store response to main program

// buttons
void goInterruptFunction();
void stopInterruptFunction();

Buttons buttons(goInterruptFunction, stopInterruptFunction);

void goInterruptFunction() {
    buttons.interrupt(Buttons::GO);
}
void stopInterruptFunction() {
    buttons.interrupt(Buttons::STOP);
}

// motor interface
void leftEncoderInterruptFunction();
void rightEncoderInterruptFunction();

MotorInterface motorInterface(leftEncoderInterruptFunction, rightEncoderInterruptFunction);

void rightEncoderInterruptFunction() {
    motorInterface.encoderInterrupt(RIGHT);
}
void leftEncoderInterruptFunction() {
    motorInterface.encoderInterrupt(LEFT);
}

// movement interface
MovementInterface movementInterface(&motorInterface, &buttons);

// calibrator
Calibrator calibrator(&movementInterface, &buttons);

// obstacle finder
ObstacleFinder obstacleFinder;

void setup() {
    //initialize led pins
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
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
    buttons.init();
    //Serial1.write("1f0\r");
    //delayMicroseconds(500);
    //Serial1.write("2f0\r");

    //send READY byte
    Serial.write('1');

}

void loop() {
    if (buttons.getStopState() != '1') {
        command = "";
        value = "";
        String *addTo = &command;  // which information we are reading from serial
        //ButtonStates();
        if (Serial.available()) {
            while (Serial.available() > 0) {
                char readIn = (char) Serial.read();
                if (readIn == '\n') {
                    break;
                } else if (readIn == '|') {
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
            response = interpretCommand(command, value);
            Serial.println(response); //sends response with \n at the end
        }
        //small delay
        delay(20);
    }
    else{
        Serial.println("STOPPED");
        delay(20);
    }
}


String interpretCommand(String command, String value) {
#ifdef VERBOSE
    Serial.print("interpreting command: ");
    Serial.print(command);
    if (value.length() > 0)  // not empty
    {
        Serial.print('|');
        Serial.print(value);
    }
    Serial.print('\n');
#endif
    String responseString = "\tn";  // what this function returns
    // tab to signal end of debug messages, n signals invalid command
    String returnString = "";     // holds the return value of the command function
    const String responseHeader = "\t1";  // tab signals that debug messages are done (don't use tab in debug messages)

    // commands to read odometers
    if (command == "<") {
        returnString = "";
        responseString = responseHeader;
        responseString += motorInterface.readEncoder(LEFT);
    } else if (command == ">") {
        returnString = "";
        responseString = responseHeader;
        responseString += motorInterface.readEncoder(RIGHT);
    }

/*
    // commands to read global coordinates
    if (command == "{") {
        returnString = "";
        responseString = responseHeader;
        responseString += String(movementInterface.global.coordinates.x[LEFT]) + ' ' +
                          String(movementInterface.global.coordinates.y[LEFT]);
    }
    else if (command == "}") {
        returnString = "";
        responseString = responseHeader;
        responseString += String(movementInterface.global.coordinates.x[RIGHT]) + ' ' +
                          String(movementInterface.global.coordinates.y[RIGHT]);
    }
*/

    // commands to calibrate sensors
    if (command == "[") {
        calibrator.getLeftCalibrationValuesForIRSensors();
        returnString = "1";
        responseString = responseHeader + returnString;
    } else if (command == "]") {
        calibrator.getRightCalibrationValuesForIRSensors();
        returnString = "1";
        responseString = responseHeader + returnString;
    } else if (command == "v") {
        calibrator.calibrateBackSensors();
        returnString = "1";
        responseString = responseHeader + returnString;
    } else if (command == "@") {
        obstacleFinder.calibrateThreshold();
        returnString = "1";
        responseString = responseHeader + returnString;
    }

        // motor stuff
    else if (command == "f") {
        movementInterface.go(FORWARD);
        returnString = "1";
        responseString = responseHeader;
        responseString += returnString;
    } else if (command == "l") {
        movementInterface.go(LEFT);
        returnString = "1";
        responseString = responseHeader;
        responseString += returnString;
    } else if (command == "r") {
        movementInterface.go(RIGHT);
        returnString = "1";
        responseString = responseHeader;
        responseString += returnString;
    } else if (command == "c") {  // calibrate
        if (value == "")
            returnString = "0";  // returnString = calibrateWithSwitches();
        else
            returnString = calibrator.calibrateWithIR(value);
        responseString = responseHeader;
        responseString += returnString;
    }

        // 7 segment stuff
    else if (command == "N") {
        // do 7 segment stuff
        displayDigit(value.toInt());
        responseString = responseHeader;
        responseString += value;
    }

        // buttons
    else if (command == "B") {
        if (value == "G") {
            responseString = responseHeader;
            responseString += buttons.getGoState();
        } else if (value == "S") {
            responseString = responseHeader;
            responseString += buttons.getStopState();
        }
    }

        // 8x8
    else if (command == "R") {  // ready light
        setReadyLight();
        responseString = responseHeader;
    } else if (command == "T") {  // objective tunnel
        setToOT(value.toInt());
        responseString = responseHeader;
    } else if (command == "D") {  // dead end tunnel
        setToDE(value.toInt());
        responseString = responseHeader;
    } else if (command == "E") {  // nothing (turn light off)
        setToEM(value.toInt());
        responseString = responseHeader;
    }

        // sensor stuff
        // either "e" or "S|E" for wire sensing
    else if (command == "e") {
        responseString = responseHeader;
        responseString += String(readEMF());
    } else if (command == "S") {
        if (value == "E") {
            responseString = responseHeader;
            responseString += String(readEMF());
        } else if (value == "O") {
            responseString = responseHeader;
            responseString += obstacleFinder.getReport();
        } else if (value == "F") {
            responseString = responseHeader;
            // check for foam
        }
    }

    //check if any BADs were obtained
    if (returnString == "BAD") {
        return "\tn";
    }
    return responseString;
}

// EMF SENSOR STUFF
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
            sum += pow(reading, 2);
            //sum += abs(reading);
        }
    }
    long int average = 0;
    if (count > 0)
        average = sum / count;
    return average;
}
//END OF EMF SENSOR STUFF


//START OF DISPLAY STUFF
/**
 * 7 segment for displaying number on die
 * @param dig the number to display
 */
void displayDigit(int dig) {
    for (int i = digits[dig]; i <= digits[dig] + 1; i++) {
        digitalWrite(LATCH, HIGH);
        int number = i;
        shiftOut(DATA, CLK, MSBFIRST, ~(char) number); // digitOne
        digitalWrite(LATCH, LOW);
        delay(1);
    }
}

void clearDigit() {
    digitalWrite(LATCH, HIGH);
    int number = 0;
    shiftOut(DATA, CLK, MSBFIRST, ~(char) number); // digitOne
    digitalWrite(LATCH, LOW);
    delay(1);
}

void setReadyLight() {
    matrix.setPixelColor(MATRIX_READY_LIGHT_PIXEL, 255, 255, 0);  // TODO: confirm this color is yellow
    matrix.show();
}

void setToOT(int index) {
    if (index != MATRIX_READY_LIGHT_PIXEL)  // don't change the ready light
    {
        matrix.setPixelColor(index, 255, 0, 0);
        matrix.show();
    }
}

void setToDE(int index) {
    if (index != MATRIX_READY_LIGHT_PIXEL)  // don't change the ready light
    {
        matrix.setPixelColor(index, 0, 255, 255);  // TODO: confirm this color is blue
        matrix.show();
    }
}

void setToEM(int index) {
    if (index != MATRIX_READY_LIGHT_PIXEL)  // don't change the ready light
    {
        matrix.setPixelColor(index, 0, 0, 0);
        matrix.show();
    }
}
//END OF DISPLAY STUFF
