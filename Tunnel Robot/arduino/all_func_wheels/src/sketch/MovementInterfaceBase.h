// interface for moving our robot
// (more abstract than MotorInterface)

#ifndef ALL_FUNC_WHEELS_MOVEMENTINTERFACEBASE_H
#define ALL_FUNC_WHEELS_MOVEMENTINTERFACEBASE_H

#include "Arduino.h"
#include <math.h>

#include "MotorInterfaceBase.h"
#include "MotorInterface.h"
#include "Buttons.h"

#define FORWARD 2

class MovementInterfaceBase
{
public:  // protected

    MotorInterfaceBase* motorInterface;
    Buttons* buttons;

    long startEncoderValues[MOTOR_COUNT];

    /** reset everything for a new movement */
    void reset()
    {
        startEncoderValues[LEFT] = motorInterface->readEncoder(LEFT);
        startEncoderValues[RIGHT] = motorInterface->readEncoder(RIGHT);
    }

    int motorSpeedLimit(const long& speed)
    {
        if (speed > MAX_MOTOR_POWER)
        {
            return MAX_MOTOR_POWER;
        }
        if (speed > MIN_MOTOR_POWER)
        {
            return (int)speed;
        }
        return MIN_MOTOR_POWER;
    }

public:
    MovementInterfaceBase(MotorInterfaceBase* _motorInterface, Buttons* _buttons)
    {
        motorInterface = _motorInterface;
        buttons = _buttons;
        reset();
    }

    virtual void go(const size_t& movementType) = 0;

    /**
     * nudge wheels in specified directions
     * @param leftDirection 1 or 0 or -1
     * 2 or -2 is big nudge
     * @param rightDirection 1 or 0 or -1
     * 2 or -2 is big nudge
     */
    void nudge(int leftDirection, int rightDirection)
    {
        int leftPower = MAX_MOTOR_POWER * abs(leftDirection) / 2;
        int rightPower = MAX_MOTOR_POWER * abs(rightDirection) / 2;

        motorInterface->setMotorPower(LEFT, leftPower, leftDirection);
        motorInterface->setMotorPower(RIGHT, rightPower, rightDirection);

        delay(70);

        motorInterface->setMotorPower(LEFT, 0, 1);
        motorInterface->setMotorPower(RIGHT, 0, 1);

        delay(200);  // give time to stop before doing anything else
    }

    /**
     * for side calibration
     * @param direction LEFT or RIGHT
     */
    void smallPivot(const size_t& direction)
    {
        if (direction == LEFT)
        {
            nudge(-1, 1);
        }
        else  // RIGHT
        {
            nudge(1, -1);
        }
    }
};

#endif //ALL_FUNC_WHEELS_MOVEMENTINTERFACEBASE_H
