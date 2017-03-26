#include <iostream>
#include <sstream>
#include <cmath>

#define SIM

#define MAX_MOTOR_POWER 254
#define MIN_MOTOR_POWER 90

#include "MovementInterface.h"

#include "PIDnoTime.h"


class MotorInterfaceSim : public MotorInterfaceBase, public ClassThatKeepsCoordinatesFromDistances
{
public:  // would be private in a different setting
    double distanceTraveled[MOTOR_COUNT];
    int currentPower[MOTOR_COUNT];

    /** power lost to friction */
    const double effectivePower(const size_t& which) const
    {
        switch(which)
        {
        case LEFT:
            return currentPower[LEFT] * 9.0 / 10.0;
        case RIGHT:
            return currentPower[RIGHT] * 3.0 / 5.0;
        default:
            throw;
        }
    }

public:
    /** default constructor */
    MotorInterfaceSim()
    {
        for (size_t i = 0; i < MOTOR_COUNT; ++i)
        {
            distanceTraveled[i] = 0;
            currentPower[i] = 0;
        }
        ClassThatKeepsCoordinatesFromDistances::reset();
    }

    /** getter for distance traveled */
    const long readEncoder(const size_t& which) const
    {
        if (which < MOTOR_COUNT)
            return (long)distanceTraveled[which];
        throw;
    }

    /**
     *  setter for current power
     *  direction is 1 or -1
     */
    void setMotorPower(const size_t& which, const int& howMuch, const int& direction)
    {
        if (which < MOTOR_COUNT)
            currentPower[which] = howMuch * direction;
    }

    /** getters for coordinates */
    const double& getXofWheel(const size_t& wheel) const
    {
        return coordinates.x[wheel];
    }
    const double& getYofWheel(const size_t& wheel) const
    {
        return coordinates.y[wheel];
    }

    /** elapse a given amount of time */
    void passTime(const size_t& movementType, const int& amount=1)
    {
        long distanceTraveledInThisTime[MOTOR_COUNT];
        for(size_t i = 0; i < MOTOR_COUNT; ++i)
        {
            double distanceThisMotorTravels = amount * effectivePower(i);
            distanceTraveledInThisTime[i] = (int)distanceThisMotorTravels;
            distanceTraveled[i] += distanceThisMotorTravels;
        }

        calculateNewCoordinates(distanceTraveledInThisTime);
    }

    const char* report() const
    {
        static const size_t MAX_LENGTH = 63;
        static char toReturn[MAX_LENGTH];

        std::ostringstream outSS;
        outSS << "L " << readEncoder(LEFT)
              << " x " << getXofWheel(LEFT)
              << " y " << getYofWheel(LEFT) << std::endl
              << "R " << readEncoder(RIGHT)
              << " x " << getXofWheel(RIGHT)
              << " y " << getYofWheel(RIGHT);
        std::string out = outSS.str();

        // copy output to char array
        for (size_t i = 0; i < MAX_LENGTH-1 && i < out.size(); ++i)
        {
            toReturn[i] = out[i];
        }
        // end c string with null
        toReturn[MAX_LENGTH-1] = 0;
        if (out.size() < MAX_LENGTH-1)
            toReturn[out.size()] = 0;

        // convert array to pointer
        return (char*)&toReturn;
    }
};


void testInterfaceSim()
{
    MotorInterfaceSim motorInterface;

    // for simulation 300, 450 will go straight
    motorInterface.setMotorPower(LEFT, 150, 1);
    motorInterface.setMotorPower(RIGHT, 200, 1);
    motorInterface.passTime(1);

    std::cout <<  motorInterface.report() << std::endl;

    motorInterface.setMotorPower(LEFT, 150, 1);
    motorInterface.setMotorPower(RIGHT, 90, 1);
    motorInterface.passTime(3);

    std::cout << motorInterface.report() << std::endl;

    motorInterface.setMotorPower(LEFT, 150, 1);
    motorInterface.setMotorPower(RIGHT, 200, 1);
    motorInterface.passTime(3);

    std::cout << motorInterface.report() << std::endl;
}

void testController()
{
    MotorInterfaceSim motorInterfaceSim;
    MovementInterface motorController(&motorInterfaceSim);

    motorController.go(RIGHT);
}

int main()
{
    testInterfaceSim();
    testController();

    return 0;
}
