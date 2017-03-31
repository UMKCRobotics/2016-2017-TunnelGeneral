// lots of debug messages to serial
// TODO: remove this, but only if we've done enough testing without it
#define VERBOSE

// #include "PID.h"  // TODO: is this used anywhere?

// #include "MovementInterface.h"  // old cartesian strategy
#include "MovementInterfaceMasterSlave.h"
#include "Calibrator.h"
#include "Buttons.h"
#include "ObstacleFinder.h"
#include "WireFinder.h"
#include "Display.h"

#ifndef PSTR
#define PSTR // Make Arduino Due happy
#endif

/** EMF pin is defined in WireFinder.h */

/** IR pins are defined in IRPins.h */

/** movement motor pins are defined in MotorInterface.h */

/** button pins are defined in Buttons.h */

//parsing inputs
String command; //used to store command from serial
String value; //used to store value from serial
String response; //used to store response to main program

// buttons
void goInterruptFunction();
void stopInterruptFunction();

Buttons buttons(goInterruptFunction, stopInterruptFunction);

// for some reason I can't put a digital read inside the button class
// I think it's an arduino library error
void goInterruptFunction() {
    int pinRead = digitalRead(GoPin);
    if (pinRead == LOW)
    {
        buttons.lowInterrupt(Buttons::GO);
    }
    else if (pinRead == HIGH)
    {
        buttons.highInterrupt(Buttons::GO);
    }
}
void stopInterruptFunction() {
    int pinRead = digitalRead(StopPin);
    if (pinRead == LOW)
    {
        buttons.lowInterrupt(Buttons::STOP);
    }
    else if (pinRead == HIGH)
    {
        buttons.highInterrupt(Buttons::STOP);
    }
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
    Display::init();

    Serial.begin(115200);

    motorInterface.encoderInit();
    motorInterface.motorInit();

    buttons.init();

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
        delay(3000);
    }
}

String power_on_self_test() {
    if (obstacleFinder.test_ir() &&
        true)  // just in case I want to add anything else
    {
        Display::setReadyLightGood();
        return "1";
    }
    Display::setReadyLightBad();
    return 0;
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
        Display::displayDigit(value.toInt());
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
        returnString = power_on_self_test();
        responseString = responseHeader + returnString;
    } else if (command == "T") {  // objective tunnel
        Display::setToOT((uint16_t)value.toInt());
        responseString = responseHeader;
    } else if (command == "D") {  // dead end tunnel
        Display::setToDE((uint16_t)value.toInt());
        responseString = responseHeader;
    } else if (command == "E") {  // nothing (turn light off)
        Display::setToEM((uint16_t)value.toInt());
        responseString = responseHeader;
    }

        // sensor stuff
        // either "e" or "S|E" for wire sensing
    else if (command == "e") {
        responseString = responseHeader;
        responseString += String(WireFinder::read());
    } else if (command == "S") {
        if (value == "E") {
            responseString = responseHeader;
            responseString += String(WireFinder::read());
        } else if (value == "O") {
            responseString = responseHeader;
            responseString += obstacleFinder.getReport();
        } else if (value == "F") {
            responseString = responseHeader;
            // check for foam
        }
    }
    else if (command == "P") {  // test bad ready light color
        Display::setReadyLightBad();
        responseString = responseHeader;
    }

    //check if any BADs were obtained
    if (returnString == "BAD") {
        return "\tn";
    }
    return responseString;
}

