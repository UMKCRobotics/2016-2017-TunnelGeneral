// handles position calibration using infrared sensors on the wood railing

#ifndef ALL_FUNC_WHEELS_CALIBRATOR_H
#define ALL_FUNC_WHEELS_CALIBRATOR_H

#include "Arduino.h"
#include "MiscDefinitions.h"
#include "IRPins.h"
#include "MovementInterfaceBase.h"

#define SAMPLE_COUNT 300  // the number of sensor samples to average together

#define BACK_CALIBRATION_THRESHOLD 3
#define THRESHOLD_FOR_BIG_NUDGE 20

#define SIDE_PIVOT_THRESHOLD 6
#define THRESHOLD_FOR_SIDE_DISTANCE 100

class Calibrator
{
public:  // private
    MovementInterfaceBase* movementInterface;

    // what to add to the difference between the two IR sensors on each side for straight to be zero
    int leftCalibrationOffset;
    int rightCalibrationOffset;

    // how far the sides should be from the wood
    int goodDistanceForLeft;
    int goodDistanceForRight;

    // how far each of the back sensors should be from the wood
    int backLeftCalibrated;
    int backRightCalibrated;

    int getIRValue(pin whichPin)
    {
        long total = 0;
        for (int i = SAMPLE_COUNT; i > 0; --i)
        {
            total += analogRead(whichPin);
        }

        return (int)(total / SAMPLE_COUNT);
    }

    int getDifferenceBetweenIRs(pin IRPinLeftOfWheel, pin IRPinRightOfWheel, int differenceOffsetForThisSide)
    {
        int leftReading = getIRValue(IRPinLeftOfWheel);
        int rightReading = getIRValue(IRPinRightOfWheel);
        return leftReading - rightReading + differenceOffsetForThisSide;
    }

    bool sideCalibrationPivotIR(pin IRPinLeftOfWheel,
                                pin IRPinRightOfWheel,
                                int differenceOffsetForThisSide,
                                int goodDistanceForThisSide)
    {
        int difference;

        while (abs(difference = getDifferenceBetweenIRs(IRPinLeftOfWheel,
                                                        IRPinRightOfWheel,
                                                        differenceOffsetForThisSide)) > SIDE_PIVOT_THRESHOLD)
        {
            if (difference > 0)  // left ir sensor is closer to wall
            {
                movementInterface->smallPivot(RIGHT);
            }
            else  // difference negative, right closer to wall
            {
                movementInterface->smallPivot(LEFT);
            }
        }

        // if too close or too far away,
        // we need to send a message back to pi to tell it that we need to fix distance from side

        int distance = (getIRValue(IRPinLeftOfWheel) + getIRValue(IRPinRightOfWheel)) / 2;
        return (abs(distance - goodDistanceForThisSide) < THRESHOLD_FOR_SIDE_DISTANCE);
    }

    void backCalibrationIR()
    {
        int leftReading = getIRValue(IR_BL);
        int rightReading = getIRValue(IR_BR);

        boolean finished = false;
        boolean leftGood;
        boolean rightGood;

        int needToMoveLeft;
        int needToMoveRight;

        while (! finished)
        {
            Serial.print("found left back at ");
            Serial.print(leftReading);
            Serial.print(" when we want ");
            Serial.println(backLeftCalibrated);

            if (leftReading - BACK_CALIBRATION_THRESHOLD > backLeftCalibrated)
            {
                // left back wheel too close
                needToMoveLeft = 1 + (leftReading - backLeftCalibrated > THRESHOLD_FOR_BIG_NUDGE);
                leftGood = false;
            }
            else if (leftReading + BACK_CALIBRATION_THRESHOLD < backLeftCalibrated)
            {
                // left back wheel too far
                needToMoveLeft = -1 - (backLeftCalibrated - leftReading > THRESHOLD_FOR_BIG_NUDGE);
                leftGood = false;
            }
            else
            {
                // within threshold
                needToMoveLeft = 0;
                leftGood = true;
            }

            Serial.print("needToMoveLeft ");
            Serial.println(needToMoveLeft);


            Serial.print("found right back at ");
            Serial.print(rightReading);
            Serial.print(" when we want ");
            Serial.println(backRightCalibrated);

            if (rightReading - BACK_CALIBRATION_THRESHOLD > backRightCalibrated)
            {
                // right back wheel too close
                needToMoveRight = 1 + (rightReading - backRightCalibrated > THRESHOLD_FOR_BIG_NUDGE);
                rightGood = false;
            }
            else if (rightReading + BACK_CALIBRATION_THRESHOLD < backRightCalibrated)
            {
                // right back wheel too far
                needToMoveRight = -1 - (backRightCalibrated - rightReading > THRESHOLD_FOR_BIG_NUDGE);
                rightGood = false;
            }
            else
            {
                // within threshold
                needToMoveRight = 0;
                rightGood = true;
            }

            Serial.print("needToMoveRight ");
            Serial.println(needToMoveRight);

            movementInterface->nudge(needToMoveLeft, needToMoveRight);

            if (rightGood && leftGood)
            {
                finished = true;
            }

            leftReading = getIRValue(IR_BL);
            rightReading = getIRValue(IR_BR);
        }
    }

public:
    Calibrator(MovementInterfaceBase* _movementInterface)
    {
        movementInterface = _movementInterface;
    }

    void getLeftCalibrationValuesForIRSensors()
    {
        leftCalibrationOffset = 0 - getDifferenceBetweenIRs(IR_L1, IR_L2, 0);

        Serial.print("left IR sensors difference offset set to: ");
        Serial.println(leftCalibrationOffset);

        goodDistanceForLeft = (getIRValue(IR_L1) + getIRValue(IR_L2)) / 2;

        Serial.print("good distance for left set to ");
        Serial.println(goodDistanceForLeft);
    }

    void getRightCalibrationValuesForIRSensors()
    {
        rightCalibrationOffset = 0 - getDifferenceBetweenIRs(IR_R2, IR_R1, 0);

        Serial.print("right IR sensors difference offset set to: ");
        Serial.println(rightCalibrationOffset);

        goodDistanceForRight = (getIRValue(IR_R1) + getIRValue(IR_R2)) / 2;

        Serial.print("good distance for right set to ");
        Serial.println(goodDistanceForRight);
    }

    /**
     *  find the sensor values for the good distance from the wood
     */
    void calibrateBackSensors()
    {
        backLeftCalibrated = getIRValue(IR_BL);
        backRightCalibrated = getIRValue(IR_BR);

        Serial.print("back calibration values: ");
        Serial.print(backLeftCalibrated);
        Serial.print(' ');
        Serial.println(backRightCalibrated);
    }

    String calibrateWithIR(String side) {
        //if L, use IR on left side
        if (side == "L")
            return String((int)sideCalibrationPivotIR(IR_L1, IR_L2, leftCalibrationOffset, goodDistanceForLeft));
            //if R, use IR on right side
        else if (side == "R")
            return String((int)sideCalibrationPivotIR(IR_R2, IR_R1, rightCalibrationOffset, goodDistanceForRight));
            //if B, use IR on back side
        else if (side == "B") {
            backCalibrationIR();
            return "1";
        }
        /*
        //if F, use IR on front side (might not use)
        else if (side == "F")
            runCalibrationPivotIR(IR_F1,IR_F2,0,threshold);
        */
        else  //signal if bad side received
            return "BAD";
    }
};

#endif //ALL_FUNC_WHEELS_CALIBRATOR_H
