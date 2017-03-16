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

//encoder pins (PINAx MUST be interrupts)
//NOTE: MOTOR1 is RIGHT, MOTOR2 is LEFT
#define RIGHT_ENCODER_INTERRUPT_PIN 2  // "pin A"
#define RIGHT_ENCODER_DIGITAL_PIN 4  // "pin B"
#define LEFT_ENCODER_INTERRUPT_PIN 3
#define LEFT_ENCODER_DIGITAL_PIN 5

//motor control
#define RIGHT_MOTOR_PIN1 40
#define RIGHT_MOTOR_PIN2 42
#define RIGHT_MOTOR_PWM 44
#define LEFT_MOTOR_PIN1 41
#define LEFT_MOTOR_PIN2 43
#define LEFT_MOTOR_PWM 45

#define MAX_MOTOR_POWER 254

// indexes for arrays
#define LEFT 0
#define RIGHT 1
#define MOTOR_COUNT 2

// one motor consistently weaker than the other? make its number higher here
#define STARTING_POWER_NEEDED_FOR_LEFT 200
#define STARTING_POWER_NEEDED_FOR_RIGHT 200

#define WIDTH 500  // TODO: distance from left wheel to right wheel - in units that the encoder gives me
#define TWELVE_INCH_DISTANCE 8000  // TODO: in units of the encoder

#define TRAVEL_DURATION 3000  // milliseconds for one grid move
#define TRAVEL_SEGMENT_COUNT 30  // the number of segments to break the travel into
const int SEGMENT_DURATION = TRAVEL_DURATION / TRAVEL_SEGMENT_COUNT;

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
    virtual const long readEncoder(const size_t& which) const = 0;
    virtual void setMotorPower(const size_t& which, const int& howMuch) = 0;
    virtual void passTime(const int& amount) = 0;
    virtual const char* report() const = 0;
};

#ifndef SIM
class MotorInterface : public MotorInterfaceBase
{
public:  // private
    volatile long odometers[MOTOR_COUNT];
    uint8_t pwm_pins[MOTOR_COUNT];
    uint8_t motor_pins_1[MOTOR_COUNT];
    uint8_t motor_pins_2[MOTOR_COUNT];

    volatile byte previousInterruptRead[MOTOR_COUNT];

    int direction[MOTOR_COUNT];  // 1 for forward, -1 for backward

    void (*leftEncoderInterruptFunction)();
    void (*rightEncoderInterruptFunction)();


public:
    MotorInterface(void (*_leftEncoderInterruptFunction)(),
                   void (*_rightEncoderInterruptFunction)())
    {
        odometers[LEFT] = 0;
        odometers[RIGHT] = 0;

        pwm_pins[LEFT] = LEFT_MOTOR_PWM;
        pwm_pins[RIGHT] = RIGHT_MOTOR_PWM;
        motor_pins_1[LEFT] = LEFT_MOTOR_PIN1;
        motor_pins_1[RIGHT] = RIGHT_MOTOR_PIN1;
        motor_pins_2[LEFT] = LEFT_MOTOR_PIN2;
        motor_pins_2[RIGHT] = RIGHT_MOTOR_PIN2;

        direction[LEFT] = 1;
        direction[RIGHT] = 1;

        leftEncoderInterruptFunction = _leftEncoderInterruptFunction;
        rightEncoderInterruptFunction = _rightEncoderInterruptFunction;
    }

    void encoderInit()
    {
        pinMode(RIGHT_ENCODER_DIGITAL_PIN, INPUT);
        pinMode(LEFT_ENCODER_DIGITAL_PIN, INPUT);

        attachInterrupt(digitalPinToInterrupt(RIGHT_ENCODER_INTERRUPT_PIN), *rightEncoderInterruptFunction, CHANGE);
        attachInterrupt(digitalPinToInterrupt(LEFT_ENCODER_INTERRUPT_PIN), *leftEncoderInterruptFunction, CHANGE);
    }

    void motorInit()
    {
        pinMode(RIGHT_MOTOR_PIN1, OUTPUT);
        pinMode(RIGHT_MOTOR_PIN2, OUTPUT);
        pinMode(RIGHT_MOTOR_PWM, OUTPUT);
        pinMode(LEFT_MOTOR_PIN1, OUTPUT);
        pinMode(LEFT_MOTOR_PIN2, OUTPUT);
        pinMode(LEFT_MOTOR_PWM, OUTPUT);
    }

    void encoderInterrupt(const size_t& which)
    {
        odometers[which] += direction[which];
    }

    const long readEncoder(const size_t& which) const
    {
        return odometers[which];
    }

    void setMotorPower(const size_t& which, const int& howMuch)
    {
        changeDirection(which, howMuch >= 0);
        analogWrite(pwm_pins[which], howMuch);
    }

    void changeDirection(const size_t& which, bool forward) {
        if (forward) {
            direction[which] = 1;
            digitalWrite(motor_pins_1[which], HIGH);
            digitalWrite(motor_pins_2[which], LOW);
        }
        else {  // backward
            direction[which] = -1;
            digitalWrite(motor_pins_1[which], LOW);
            digitalWrite(motor_pins_2[which], HIGH);
        }
    }


    /** elapse the amount of time we want to take to travel twelve inches * amount / segment count */
    void passTime(const int& amount)
    {
        long stopTime = millis() + (SEGMENT_DURATION * amount);
        while (millis() < stopTime) ;
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

        travelTimeCount = TRAVEL_SEGMENT_COUNT;

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
            return ((TRAVEL_SEGMENT_COUNT - travelTimeCount) * 1.3333 / TRAVEL_SEGMENT_COUNT) + 0.3333;
        }
        // care more about x at the beginning of the move, less at the end
        // from 1.67 to 0
        return 1.6667 * travelTimeCount / TRAVEL_SEGMENT_COUNT;
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

    void turnLeft()
    {
#ifndef SIM
        Serial.println("turn left function not implemented");

#endif
    }

    void turnRight()
    {
#ifndef SIM
        Serial.println("turn right function not implemented");

#endif
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

            if (progressMade[LEFT])  // > 0
            {
                powerNeeded[LEFT] = min(MAX_MOTOR_POWER, (int)round(progressNeedToMake[LEFT] * powerNeeded[LEFT]
                                                                    / progressMade[LEFT]));
            }
            // else don't change it

            if (progressMade[RIGHT])  // > 0
            {
                powerNeeded[RIGHT] = min(MAX_MOTOR_POWER, (int)round(progressNeedToMake[RIGHT] * powerNeeded[RIGHT]
                                                                     / progressMade[RIGHT]));
            }
            // else don't change it

#ifdef SIM
            std::cout << motorInterface->report() << std::endl;

#endif
        }

        motorInterface->setMotorPower(LEFT, 0);
        motorInterface->setMotorPower(RIGHT, 0);
    }
};

#endif //ALL_FUNC_WHEELS_MOTORCONTROLLER_H
