#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

#include "PIDnoTime.h"

// indexes for arrays
#define LEFT 0
#define RIGHT 1
#define MOTOR_COUNT 2

// right motor weaker than left
#define STARTING_POWER_NEEDED_FOR_LEFT 297
#define STARTING_POWER_NEEDED_FOR_RIGHT 445

#define WIDTH 500  // TODO: distance from left wheel to right wheel - in units that the encoder gives me

struct RobotCoordinates
{
    double x[MOTOR_COUNT];
    double y[MOTOR_COUNT];
};

class MotorInterface
{
public:  // would be private in a different setting
    double distanceTraveled[MOTOR_COUNT];
    int currentPower[MOTOR_COUNT];
    RobotCoordinates coordinates;

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

    void calculateNewCoordinates(const double distance[])
    {
        // http://math.stackexchange.com/questions/2183324/cartesian-coordinates-on-2-circles

        // utur = untranslated unrotated (left wheel started at 0, 0 and right wheel started at WIDTH,0)
        RobotCoordinates utur;

        // utr = untranslated rotated
        RobotCoordinates utr;

        // angle of the distance around a circle that the robot traveled (radians)
        double t = (distance[LEFT] - distance[RIGHT]) / WIDTH;

        double cos_t = std::cos(t);
        double sin_t = std::sin(t);

        // distance from the center of the circle to the right wheel
        double r;
        if (t)  // != 0
        {
            r = distance[RIGHT] / t;

            utur.x[LEFT] = (r+WIDTH) * (1 - cos_t);
            utur.y[LEFT] = (r+WIDTH) * sin_t;

            utur.x[RIGHT] = (r + WIDTH) - (r * cos_t);
            utur.y[RIGHT] = r * sin_t;
        }
        else  // robot went straight
        {
            utur.x[LEFT] = 0;
            utur.y[LEFT] = distance[LEFT];

            utur.x[RIGHT] = WIDTH;
            utur.y[RIGHT] = distance[RIGHT];
        }

        // rotate and translate onto current coordinates

        // first rotate - by the angle of the right wheel
        // we want to undo the rotation that would bring the current corrdinates back to straight,
        // so we use the clockwise (backwards) rotation matrix
        t = std::atan2(coordinates.y[LEFT] - coordinates.y[RIGHT],
                       coordinates.x[RIGHT] - coordinates.x[LEFT]);
        cos_t = std::cos(t);
        sin_t = std::sin(t);

        utr.x[LEFT] = utur.x[LEFT] * cos_t + utur.y[LEFT] * sin_t;
        utr.y[LEFT] = utur.y[LEFT] * cos_t - utur.x[LEFT] * sin_t;

        utr.x[RIGHT] = utur.x[RIGHT] * cos_t + utur.y[RIGHT] * sin_t;
        utr.y[RIGHT] = utur.y[RIGHT] * cos_t - utur.x[RIGHT] * sin_t;

        // now translate - add left wheel coordinate
        // right first, so we don't lose the left wheel coordinate
        coordinates.x[RIGHT] = utr.x[RIGHT] + coordinates.x[LEFT];
        coordinates.y[RIGHT] = utr.y[RIGHT] + coordinates.y[LEFT];

        coordinates.x[LEFT] += utr.x[LEFT];
        coordinates.y[LEFT] += utr.y[LEFT];
    }

public:
    /** default constructor */
    MotorInterface()
    {
        for (size_t i = 0; i < MOTOR_COUNT; ++i)
        {
            distanceTraveled[i] = 0;
            currentPower[i] = 0;
            coordinates.x[i] = i * WIDTH;
            coordinates.y[i] = 0;
        }
    }

    /** getter for distance traveled */
    const double& readEncoder(const size_t& which) const
    {
        if (which < MOTOR_COUNT)
            return distanceTraveled[which];
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
    void passTime(int amount)
    {
        double distanceTraveledInThisTime[MOTOR_COUNT];
        for(size_t i = 0; i < MOTOR_COUNT; ++i)
        {
            distanceTraveledInThisTime[i] = amount * effectivePower(i);
            distanceTraveled[i] += distanceTraveledInThisTime[i];
        }

        calculateNewCoordinates(distanceTraveledInThisTime);
    }

    const std::string report() const
    {
        std::ostringstream out;
        out << "L " << readEncoder(LEFT)
            << " x " << getXofWheel(LEFT)
            << " y " << getYofWheel(LEFT) << std::endl
            << "R " << readEncoder(RIGHT)
            << " x " << getXofWheel(RIGHT)
            << " y " << getYofWheel(RIGHT);
        return out.str();
    }
};

class MotorController
{
public:  // private in a different setting

    const double TWELVE_INCH_DISTANCE = 8000;  // in whatever units the encoder gives me TODO: tune this
    const int TRAVEL_TIME = 30;  // the number of segments to break the travel into

    int powerNeeded[MOTOR_COUNT];  // the power needed t to travel that distance in that many units of time

    RobotCoordinates coordinates;  // relative from where I started this group of twelve inches
    double totalMeasuredAboveTheOther[MOTOR_COUNT];  // TODO: different class
    int travelTimeCount;  // counts down
    PID<double> pid;

    MotorInterface motorInterface;
    double startEncoderValues[MOTOR_COUNT];

    void calculateNewCoordinates(const double distance[])
    {
        // http://math.stackexchange.com/questions/2183324/cartesian-coordinates-on-2-circles

        // utur = untranslated unrotated (left wheel started at 0, 0 and right wheel started at WIDTH,0)
        RobotCoordinates utur;

        // utr = untranslated rotated
        RobotCoordinates utr;

        // angle of the distance around a circle that the robot traveled (radians)
        double t = (distance[LEFT] - distance[RIGHT]) / WIDTH;

        double cos_t = std::cos(t);
        double sin_t = std::sin(t);

        // distance from the center of the circle to the right wheel
        double r;
        if (t)  // != 0
        {
            r = distance[RIGHT] / t;

            utur.x[LEFT] = (r+WIDTH) * (1 - cos_t);
            utur.y[LEFT] = (r+WIDTH) * sin_t;

            utur.x[RIGHT] = (r + WIDTH) - (r * cos_t);
            utur.y[RIGHT] = r * sin_t;
        }
        else  // robot went straight
        {
            utur.x[LEFT] = 0;
            utur.y[LEFT] = distance[LEFT];

            utur.x[RIGHT] = WIDTH;
            utur.y[RIGHT] = distance[RIGHT];
        }

        // rotate and translate onto current coordinates

        // first rotate - by the angle of the right wheel
        // we want to undo the rotation that would bring the current corrdinates back to straight,
        // so we use the clockwise (backwards) rotation matrix
        t = std::atan2(coordinates.y[LEFT] - coordinates.y[RIGHT],
                       coordinates.x[RIGHT] - coordinates.x[LEFT]);
        cos_t = std::cos(t);
        sin_t = std::sin(t);

        utr.x[LEFT] = utur.x[LEFT] * cos_t + utur.y[LEFT] * sin_t;
        utr.y[LEFT] = utur.y[LEFT] * cos_t - utur.x[LEFT] * sin_t;

        utr.x[RIGHT] = utur.x[RIGHT] * cos_t + utur.y[RIGHT] * sin_t;
        utr.y[RIGHT] = utur.y[RIGHT] * cos_t - utur.x[RIGHT] * sin_t;

        // now translate - add left wheel coordinate
        // right first, so we don't lose the left wheel coordinate
        coordinates.x[RIGHT] = utr.x[RIGHT] + coordinates.x[LEFT];
        coordinates.y[RIGHT] = utr.y[RIGHT] + coordinates.y[LEFT];

        coordinates.x[LEFT] += utr.x[LEFT];
        coordinates.y[LEFT] += utr.y[LEFT];
    }


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

        startEncoderValues[LEFT] = motorInterface.readEncoder(LEFT);
        startEncoderValues[RIGHT] = motorInterface.readEncoder(RIGHT);
    }

    double distanceFromGoal(const size_t& which, const double& howMuchXMatters = 1)
    {
        // howMuchXMatters on a scale of 0 to 2 - default 1 is real pythagorean distance
        // pythagorean theorem distance from (0, twelve inches) or (width, twelve inches)
        double xDifference = (which * WIDTH) - coordinates.x[which];
        double yDifference = TWELVE_INCH_DISTANCE - coordinates.y[which];
        return sqrt(howMuchXMatters * xDifference*xDifference + (2-howMuchXMatters) * yDifference*yDifference);
    }

public:
    // constructor
    MotorController()
    {
        reset();
        powerNeeded[LEFT] = STARTING_POWER_NEEDED_FOR_LEFT;
        powerNeeded[RIGHT] = STARTING_POWER_NEEDED_FOR_RIGHT;
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

            previousEncoderReading[LEFT] = motorInterface.readEncoder(LEFT);
            previousEncoderReading[RIGHT] = motorInterface.readEncoder(RIGHT);

            motorInterface.setMotorPower(LEFT, input[LEFT]);
            motorInterface.setMotorPower(RIGHT, input[RIGHT]);

            motorInterface.passTime(1);
            --travelTimeCount;

            newEncoderReading[LEFT] = motorInterface.readEncoder(LEFT);
            newEncoderReading[RIGHT] = motorInterface.readEncoder(RIGHT);

            distancesTraveledThisTime[LEFT] = newEncoderReading[LEFT] - previousEncoderReading[LEFT];
            distancesTraveledThisTime[RIGHT] = newEncoderReading[RIGHT] - previousEncoderReading[RIGHT];

            calculateNewCoordinates(distancesTraveledThisTime);
            calculateDistanceAheadOfOther();

            std::cout << motorInterface.report() << std::endl;
        }
    }

    void goForward()
    {
        reset();

        double previousEncoderReading[MOTOR_COUNT];
        double newEncoderReading[MOTOR_COUNT];
        double distancesTraveledThisTime[MOTOR_COUNT];
        double previousDistanceFromGoal[MOTOR_COUNT];
        double newDistanceFromGoal[MOTOR_COUNT];
        double progressMade[MOTOR_COUNT];
        double progressNeedToMake[MOTOR_COUNT];

        double howMuchWeCareAboutXThisTime;

        while (travelTimeCount > 0)
        {
            std::cout << "time left " << travelTimeCount
                      << " input left " << powerNeeded[LEFT] << " right " << powerNeeded[RIGHT]
                      << std::endl;

            howMuchWeCareAboutXThisTime = 1.66 * travelTimeCount / TRAVEL_TIME;
            // 1.5 at beginning, down to almost 0 at the end

            previousEncoderReading[LEFT] = motorInterface.readEncoder(LEFT);
            previousEncoderReading[RIGHT] = motorInterface.readEncoder(RIGHT);

            previousDistanceFromGoal[LEFT] = distanceFromGoal(LEFT, howMuchWeCareAboutXThisTime);
            previousDistanceFromGoal[RIGHT] = distanceFromGoal(RIGHT, howMuchWeCareAboutXThisTime);

            progressNeedToMake[LEFT] = previousDistanceFromGoal[LEFT] / travelTimeCount;
            progressNeedToMake[RIGHT] = previousDistanceFromGoal[RIGHT] / travelTimeCount;

            motorInterface.setMotorPower(LEFT, powerNeeded[LEFT]);
            motorInterface.setMotorPower(RIGHT, powerNeeded[RIGHT]);

            motorInterface.passTime(1);
            --travelTimeCount;

            newEncoderReading[LEFT] = motorInterface.readEncoder(LEFT);
            newEncoderReading[RIGHT] = motorInterface.readEncoder(RIGHT);

            distancesTraveledThisTime[LEFT] = newEncoderReading[LEFT] - previousEncoderReading[LEFT];
            distancesTraveledThisTime[RIGHT] = newEncoderReading[RIGHT] - previousEncoderReading[RIGHT];

            calculateNewCoordinates(distancesTraveledThisTime);

            newDistanceFromGoal[LEFT] = distanceFromGoal(LEFT, howMuchWeCareAboutXThisTime);
            newDistanceFromGoal[RIGHT] = distanceFromGoal(RIGHT, howMuchWeCareAboutXThisTime);

            progressMade[LEFT] = previousDistanceFromGoal[LEFT] - newDistanceFromGoal[LEFT];
            progressMade[RIGHT] = previousDistanceFromGoal[RIGHT] - newDistanceFromGoal[RIGHT];

            powerNeeded[LEFT] = (int)round(progressNeedToMake[LEFT] * powerNeeded[LEFT] / progressMade[LEFT]);
            powerNeeded[RIGHT] = (int)round(progressNeedToMake[RIGHT] * powerNeeded[RIGHT] / progressMade[RIGHT]);

            std::cout << motorInterface.report() << std::endl;
        }

        motorInterface.setMotorPower(LEFT, 0);
        motorInterface.setMotorPower(RIGHT, 0);
    }
};

void testInterfaceSim()
{
    MotorInterface motorInterface;

    // for simulation 300, 450 will go straight
    motorInterface.setMotorPower(LEFT, 300);
    motorInterface.setMotorPower(RIGHT, 450);
    motorInterface.passTime(1);

    std::cout << motorInterface.report() << std::endl;

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
    MotorController motorController;

    motorController.goForward();
}

int main()
{
    testInterfaceSim();
    // testController();

    return 0;
}
