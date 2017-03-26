// interface for moving our robot
// (more abstract than MotorInterface)

#ifndef ALL_FUNC_WHEELS_MOVEMENTINTERFACEBASE_H
#define ALL_FUNC_WHEELS_MOVEMENTINTERFACEBASE_H

#include "Arduino.h"
#include <math.h>

#include "MotorInterfaceBase.h"
#include "MotorInterface.h"

#define FORWARD 2

class MovementInterfaceBase
{
public:  // private

    MotorInterfaceBase* motorInterface;
    long startEncoderValues[MOTOR_COUNT];

    /** reset everything for a new movement */
    void reset()
    {
        startEncoderValues[LEFT] = motorInterface->readEncoder(LEFT);
        startEncoderValues[RIGHT] = motorInterface->readEncoder(RIGHT);
    }

    int motorSpeedLimit(const int& speed)
    {
        if (speed > MAX_MOTOR_POWER)
        {
            return MAX_MOTOR_POWER;
        }
        if (speed > MIN_MOTOR_POWER)
        {
            return speed;
        }
        return MIN_MOTOR_POWER;
    }

public:
    MovementInterfaceBase(MotorInterfaceBase* _motorInterface)
    {
        motorInterface = _motorInterface;
        reset();
    }

    virtual void go(const size_t& movementType) = 0;


    /**
     * for side calibration
     * @param direction LEFT or RIGHT
     */
    void smallPivot(const size_t& direction)
    {
        motorInterface->setMotorPower(direction, MAX_MOTOR_POWER, -1);
        motorInterface->setMotorPower((direction + 1) % 2, MAX_MOTOR_POWER, 1);

        delay(100);

        motorInterface->setMotorPower(direction, 0, 1);
        motorInterface->setMotorPower((direction + 1) % 2, 0, 1);

        delay(150);  // give time to stop before doing anything else
    }

    /**
     * nudge one wheel in a certain direction
     * @param whichWheel LEFT or RIGHT
     * @param direction 1 or 0 or -1
     */
    void nudge(int leftDirection, int rightDirection)
    {
        int leftPower = MAX_MOTOR_POWER * abs(leftDirection);
        int rightPower = MAX_MOTOR_POWER * abs(rightDirection);

        motorInterface->setMotorPower(LEFT, leftPower, leftDirection);
        motorInterface->setMotorPower(RIGHT, rightPower, rightDirection);

        delay(100);

        motorInterface->setMotorPower(LEFT, 0, 1);
        motorInterface->setMotorPower(RIGHT, 0, 1);

        delay(150);
    }
};

#endif //ALL_FUNC_WHEELS_MOVEMENTINTERFACEBASE_H
