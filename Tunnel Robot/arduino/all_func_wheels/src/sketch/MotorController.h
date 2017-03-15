#ifndef ALL_FUNC_WHEELS_MOTORCONTROLLER_H
#define ALL_FUNC_WHEELS_MOTORCONTROLLER_H


#ifdef SIM
    #include <iostream>
    #include <cmath>
    #include <cstddef>

    #define cos std::cos
    #define sin std::sin
    #define atan2 std::atan2

#endif

#ifndef SIM
    #include "Arduino.h"
    #include <math.h>

#endif

// indexes for arrays
#define LEFT 0
#define RIGHT 1
#define MOTOR_COUNT 2

// right motor weaker than left
#define STARTING_POWER_NEEDED_FOR_LEFT 297
#define STARTING_POWER_NEEDED_FOR_RIGHT 445

#define WIDTH 500  // TODO: distance from left wheel to right wheel - in units that the encoder gives me
#define TWELVE_INCH_DISTANCE 8000  // TODO: in units of the encoder

#define TRAVEL_TIME 30  // the number of segments to break the travel into

struct RobotCoordinates
{
    double x[MOTOR_COUNT];
    double y[MOTOR_COUNT];
};

class ClassThatKeepsCoordinatesFromDistances
{
public:  // protected
    RobotCoordinates coordinates;

    /**
     *  update coordinates according to the distances that the encoders say that we traveled
     *  distance is array of distances that the encoders say that we traveled
     */
    void calculateNewCoordinates(const double distance[])
    {
        // http://math.stackexchange.com/questions/2183324/cartesian-coordinates-on-2-circles

        // utur = untranslated unrotated (left wheel started at 0, 0 and right wheel started at WIDTH,0)
        RobotCoordinates utur;

        // utr = untranslated rotated
        RobotCoordinates utr;

        // angle of the distance around a circle that the robot traveled (radians)
        double t = (distance[LEFT] - distance[RIGHT]) / WIDTH;

        double cos_t = cos(t);
        double sin_t = sin(t);

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
        // we want to undo the rotation that would bring the current coordinates back to straight,
        // so we use the clockwise (backwards) rotation matrix
        t = atan2(coordinates.y[LEFT] - coordinates.y[RIGHT],
                  coordinates.x[RIGHT] - coordinates.x[LEFT]);
        cos_t = cos(t);
        sin_t = sin(t);

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

    virtual void reset()
    {
        coordinates.x[LEFT] = 0;
        coordinates.y[LEFT] = 0;
        coordinates.x[RIGHT] = WIDTH;
        coordinates.y[RIGHT] = 0;
    }
};

class MotorInterfaceBase
{
public:
    virtual const int readEncoder(const size_t& which) const = 0;
    virtual void setMotorPower(const size_t& which, const int& howMuch) = 0;
    virtual void passTime(const int& amount) = 0;
    virtual const char* report() const = 0;
};

#ifndef SIM
class MotorInterface : public MotorInterfaceBase
{
public:  // private
    volatile int* odometers[MOTOR_COUNT];
    uint8_t pins[MOTOR_COUNT];
public:
    MotorInterface(volatile int* leftOdometer,
                   volatile int* rightOdometer,
                   const uint8_t& leftAnalogPin,
                   const uint8_t& rightAnalogPin)
    {
        odometers[LEFT] = leftOdometer;
        odometers[RIGHT] = rightOdometer;
        pins[LEFT] = leftAnalogPin;
        pins[RIGHT] = rightAnalogPin;
    }

    const int readEncoder(const size_t& which) const
    {
        return *(odometers[which]);
    }

    void setMotorPower(const size_t& which, const int& howMuch)
    {
        analogWrite(pins[which], howMuch);
    }

    /** elapse the amount of time we want to take to travel twelve inches / segment count */
    void passTime(const int& amount)
    {
        // TODO: implement passTime
    }

    const char* report() const
    {
        return "";
    }
};

#endif

class MotorController : public ClassThatKeepsCoordinatesFromDistances
{
public:  // private in a different setting

    int powerNeeded[MOTOR_COUNT];  // the power needed to travel twelve inches / number of segments

    // inherited
    // RobotCoordinates coordinates;  // relative from where I started this movement

    int travelTimeCount;  // counts down

    MotorInterfaceBase* motorInterface;
    double startEncoderValues[MOTOR_COUNT];

    /** reset everything for a new movement */
    void reset()
    {
        coordinates.x[LEFT] = 0;
        coordinates.y[LEFT] = 0;
        coordinates.x[RIGHT] = WIDTH;
        coordinates.y[RIGHT] = 0;

        travelTimeCount = TRAVEL_TIME;

        startEncoderValues[LEFT] = motorInterface->readEncoder(LEFT);
        startEncoderValues[RIGHT] = motorInterface->readEncoder(RIGHT);
    }

    double distanceFromGoal(const size_t& which, const double& howMuchXMatters = 1)
    {
        // pythagorean theorem distance from (0, twelve inches) or (width, twelve inches)

        // howMuchXMatters on a scale of 0 to 2 - default 1 is real pythagorean distance

        double xDifference = (which * WIDTH) - coordinates.x[which];
        double yDifference = TWELVE_INCH_DISTANCE - coordinates.y[which];
        return sqrt(howMuchXMatters * xDifference*xDifference + (2-howMuchXMatters) * yDifference*yDifference);
    }

    /**
     *  on a scale of 0 to 2, how much we care about x axis in movement
     *  movement is LEFT (0) or RIGHT (1) or anything else being forward
     */
    double howMuchToCareAboutX(int movement)
    {
        if (movement == LEFT || movement == RIGHT)
        {
            // care more about y at the beginning and more about x later
            // from .33 to 1.67
            return ((TRAVEL_TIME - travelTimeCount) * 1.3333 / TRAVEL_TIME) + 0.3333;
        }
        // care more about x at the beginning of the move, less at the end
        // from 1.67 to 0
        return 1.6667 * travelTimeCount / TRAVEL_TIME;
    }

public:
    // constructor
    MotorController(MotorInterfaceBase* _motorInterface)
    {
        motorInterface = _motorInterface;
        reset();
        powerNeeded[LEFT] = STARTING_POWER_NEEDED_FOR_LEFT;
        powerNeeded[RIGHT] = STARTING_POWER_NEEDED_FOR_RIGHT;
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
            /*
            Serial.print("time left ");
            Serial.print(travelTimeCount);
            Serial.print(" input left ");
            Serial.print(powerNeeded[LEFT]);
            Serial.print(" right ");
            Serial.print(powerNeeded[RIGHT]);
            Serial.print('\n');
            */

            howMuchWeCareAboutXThisTime = howMuchToCareAboutX(2);

            previousEncoderReading[LEFT] = motorInterface->readEncoder(LEFT);
            previousEncoderReading[RIGHT] = motorInterface->readEncoder(RIGHT);

            previousDistanceFromGoal[LEFT] = distanceFromGoal(LEFT, howMuchWeCareAboutXThisTime);
            previousDistanceFromGoal[RIGHT] = distanceFromGoal(RIGHT, howMuchWeCareAboutXThisTime);

            progressNeedToMake[LEFT] = previousDistanceFromGoal[LEFT] / travelTimeCount;
            progressNeedToMake[RIGHT] = previousDistanceFromGoal[RIGHT] / travelTimeCount;

            motorInterface->setMotorPower(LEFT, powerNeeded[LEFT]);
            motorInterface->setMotorPower(RIGHT, powerNeeded[RIGHT]);

            motorInterface->passTime(1);
            --travelTimeCount;

            newEncoderReading[LEFT] = motorInterface->readEncoder(LEFT);
            newEncoderReading[RIGHT] = motorInterface->readEncoder(RIGHT);

            distancesTraveledThisTime[LEFT] = newEncoderReading[LEFT] - previousEncoderReading[LEFT];
            distancesTraveledThisTime[RIGHT] = newEncoderReading[RIGHT] - previousEncoderReading[RIGHT];

            calculateNewCoordinates(distancesTraveledThisTime);

            newDistanceFromGoal[LEFT] = distanceFromGoal(LEFT, howMuchWeCareAboutXThisTime);
            newDistanceFromGoal[RIGHT] = distanceFromGoal(RIGHT, howMuchWeCareAboutXThisTime);

            progressMade[LEFT] = previousDistanceFromGoal[LEFT] - newDistanceFromGoal[LEFT];
            progressMade[RIGHT] = previousDistanceFromGoal[RIGHT] - newDistanceFromGoal[RIGHT];

            powerNeeded[LEFT] = (int)round(progressNeedToMake[LEFT] * powerNeeded[LEFT] / progressMade[LEFT]);
            powerNeeded[RIGHT] = (int)round(progressNeedToMake[RIGHT] * powerNeeded[RIGHT] / progressMade[RIGHT]);

#ifdef SIM
            std::cout << motorInterface->report() << std::endl;

#endif
        }

        motorInterface->setMotorPower(LEFT, 0);
        motorInterface->setMotorPower(RIGHT, 0);
    }
};

#endif //ALL_FUNC_WHEELS_MOTORCONTROLLER_H
