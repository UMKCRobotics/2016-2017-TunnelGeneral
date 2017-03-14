#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

#define SIM
#include "MotorController.h"

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
    const int readEncoder(const size_t& which) const
    {
        if (which < MOTOR_COUNT)
            return (int)round(distanceTraveled[which]);
        throw;
    }

    /** setter for current power */
    void setMotorPower(const size_t& which, const int& power)
    {
        if (which < MOTOR_COUNT)
            currentPower[which] = power;
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
    void passTime(const int& amount)
    {
        double distanceTraveledInThisTime[MOTOR_COUNT];
        for(size_t i = 0; i < MOTOR_COUNT; ++i)
        {
            distanceTraveledInThisTime[i] = amount * effectivePower(i);
            distanceTraveled[i] += distanceTraveledInThisTime[i];
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
        for (size_t i = 0; i < MAX_LENGTH && i < out.size(); ++i)
        {
            toReturn[i] = out[i];
        }

        // convert array to pointer
        return (char*)&toReturn;
    }
};

class MotorControllerOld : public ClassThatKeepsCoordinatesFromDistances
{
public:  // private

    double totalMeasuredAboveTheOther[MOTOR_COUNT];
    int travelTimeCount;  // counts down
    PID<double> pid;

    MotorInterfaceBase* motorInterface;
    double startEncoderValues[MOTOR_COUNT];

    void calculateDistanceAheadOfOther() {
        if (coordinates.y[LEFT] > coordinates.y[RIGHT])
            totalMeasuredAboveTheOther[LEFT] += coordinates.y[LEFT] - coordinates.y[RIGHT];
        else
            totalMeasuredAboveTheOther[RIGHT] += coordinates.y[RIGHT] - coordinates.y[LEFT];
    }


    /** shoot for making both motors ahead of each other by equal amounts */
    double differenceGoal(const size_t& which) const
    {
        if (totalMeasuredAboveTheOther[which] >= totalMeasuredAboveTheOther[(which+1)%2])
        {
            // this one has been ahead more
            return 0;
        }
        // the other one has been ahead more
        return totalMeasuredAboveTheOther[(which+1)%2] - totalMeasuredAboveTheOther[which];
    }

    /** remaining distance this motor is shooting for */
    double distanceGoal(const size_t& which) const
    {
        double realDistanceRemaining = TWELVE_INCH_DISTANCE - coordinates.y[which];
        return realDistanceRemaining + realDistanceRemaining * differenceGoal(which) / TWELVE_INCH_DISTANCE;
    }

    /** reset everything for a new movement */
    void reset()
    {
        coordinates.x[LEFT] = 0;
        coordinates.y[LEFT] = 0;
        coordinates.x[RIGHT] = WIDTH;
        coordinates.y[RIGHT] = 0;

        totalMeasuredAboveTheOther[LEFT] = 0;
        totalMeasuredAboveTheOther[RIGHT] = 0;

        travelTimeCount = TRAVEL_TIME;

        pid.reset();

        startEncoderValues[LEFT] = motorInterface->readEncoder(LEFT);
        startEncoderValues[RIGHT] = motorInterface->readEncoder(RIGHT);
    }

public:
    // constructor
    MotorControllerOld()
    {
        reset();
    }

    void goForwardOld()
    {
        reset();

        int input[MOTOR_COUNT];
        double distanceGoalForNextUnitOfTime[MOTOR_COUNT];
        double previousEncoderReading[MOTOR_COUNT];
        double newEncoderReading[MOTOR_COUNT];
        double distancesTraveledThisTime[MOTOR_COUNT];

        while (travelTimeCount > 0)
        {
            std::cout << (
            distanceGoalForNextUnitOfTime[LEFT] = distanceGoal(LEFT) / travelTimeCount
            ) << std::endl;
            std::cout << (
            distanceGoalForNextUnitOfTime[RIGHT] = distanceGoal(RIGHT) / travelTimeCount
            ) << std::endl;
            /*
            input[LEFT] = (int)pid.calculate(0, distanceGoalForNextUnitOfTime[LEFT]);
            input[RIGHT] = (int)pid.calculate(0, distanceGoalForNextUnitOfTime[RIGHT]);
            */
            input[LEFT] = (int)round(distanceGoalForNextUnitOfTime[LEFT]);
            input[RIGHT] = (int)round(distanceGoalForNextUnitOfTime[RIGHT]);

            std::cout << "time left " << travelTimeCount << " input left " << input[LEFT] << " right " << input[RIGHT]
                                                                                                       << std::endl;

            previousEncoderReading[LEFT] = motorInterface->readEncoder(LEFT);
            previousEncoderReading[RIGHT] = motorInterface->readEncoder(RIGHT);

            motorInterface->setMotorPower(LEFT, input[LEFT]);
            motorInterface->setMotorPower(RIGHT, input[RIGHT]);

            motorInterface->passTime(1);
            --travelTimeCount;

            newEncoderReading[LEFT] = motorInterface->readEncoder(LEFT);
            newEncoderReading[RIGHT] = motorInterface->readEncoder(RIGHT);

            distancesTraveledThisTime[LEFT] = newEncoderReading[LEFT] - previousEncoderReading[LEFT];
            distancesTraveledThisTime[RIGHT] = newEncoderReading[RIGHT] - previousEncoderReading[RIGHT];

            calculateNewCoordinates(distancesTraveledThisTime);
            calculateDistanceAheadOfOther();

            std::cout << motorInterface->report() << std::endl;
        }
    }
};

void testInterfaceSim()
{
    MotorInterfaceSim motorInterface;

    // for simulation 300, 450 will go straight
    motorInterface.setMotorPower(LEFT, 300);
    motorInterface.setMotorPower(RIGHT, 450);
    motorInterface.passTime(1);

    std::cout <<  motorInterface.report() << std::endl;

    motorInterface.setMotorPower(LEFT, 300);
    motorInterface.setMotorPower(RIGHT, 10);
    motorInterface.passTime(3);

    std::cout << motorInterface.report() << std::endl;

    motorInterface.setMotorPower(LEFT, 300);
    motorInterface.setMotorPower(RIGHT, 400);
    motorInterface.passTime(3);

    std::cout << motorInterface.report() << std::endl;
}

void testController()
{
    MotorInterfaceSim motorInterfaceSim;
    MotorController motorController(&motorInterfaceSim);

    motorController.goForward();
}

int main()
{
    testInterfaceSim();
    testController();

    return 0;
}
