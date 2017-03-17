#ifndef ALL_FUNC_WHEELS_MOTORCONTROLLER_H
#define ALL_FUNC_WHEELS_MOTORCONTROLLER_H


#ifdef SIM
    #include <iostream>
    #include <cmath>      // sin cos atan2 sqrt
    #include <cstddef>    // uint8_t
    #include <cstdlib>    // abs
    #include <algorithm>  // min

    #define cos std::cos
    #define sin std::sin
    #define atan2 std::atan2
    #define MIN std::min

#endif

#ifndef SIM
    #include "Arduino.h"
    #include <math.h>

    #define MIN min

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

#define MIN_MOTOR_POWER 90
#define MAX_MOTOR_POWER 254

// indexes for arrays
#define LEFT 0
#define RIGHT 1
#define MOTOR_COUNT 2

#define FORWARD 2

// TODO: tune with average values in log
// one motor consistently weaker than the other? make its number higher here
#define STARTING_FORWARD_POWER_NEEDED_FOR_LEFT 150
#define STARTING_FORWARD_POWER_NEEDED_FOR_RIGHT 150
#define STARTING_TURN_POWER_NEEDED_FOR_LEFT 120  // absolute values
#define STARTING_TURN_POWER_NEEDED_FOR_RIGHT 120

// TODO: these might be able to be tuned better
// WIDTH tuned by making turns accurate 90 degrees
// TWELVE_INCH_DISTANCE tuned by making forward accurate 12 inches
#define WIDTH 1618  // distance from left wheel to right wheel - in units that the encoder gives me
#define TWELVE_INCH_DISTANCE 2427  // in units of the encoder

#define TRAVEL_SEGMENT_COUNT 30  // the number of segments to break the travel into

#define FORWARD_TRAVEL_DURATION 3000  // milliseconds for one grid move
const int FORWARD_SEGMENT_DURATION = FORWARD_TRAVEL_DURATION / TRAVEL_SEGMENT_COUNT;

#define TURN_TRAVEL_DURATION 2000  // milliseconds to turn
const int TURN_SEGMENT_DURATION = TURN_TRAVEL_DURATION / TRAVEL_SEGMENT_COUNT;

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
    void calculateNewCoordinates(const long distance[])
    {
        // http://math.stackexchange.com/questions/2183324/cartesian-coordinates-on-2-circles

        // utur = untranslated unrotated (left wheel started at 0, 0 and right wheel started at WIDTH,0)
        RobotCoordinates utur;

        // utr = untranslated rotated
        RobotCoordinates utr;

        // angle of the distance around a circle that the robot traveled (radians)
        double t = (double)(distance[LEFT] - distance[RIGHT]) / WIDTH;

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
    virtual void setMotorPower(const size_t& which, const int& howMuch, const int& direction) = 0;
    virtual void passTime(const size_t& movementType, const int& amount=1) = 0;
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

    void setMotorPower(const size_t& which, const int& howMuch, const int& direction)
    {
        changeDirection(which, direction > 0);
        analogWrite(pwm_pins[which], abs(howMuch));
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
    void passTime(const size_t& movementType, const int& amount=1)
    {
        long stopTime;
        if (movementType == FORWARD)
        {
            stopTime = millis() + (FORWARD_SEGMENT_DURATION * amount);
        }
        else  // turn
        {
            stopTime = millis() + (TURN_SEGMENT_DURATION * amount);
        }
        while (millis() < stopTime) ;
    }

    const char* report() const
    {
        return "";
    }
};

#endif

class GlobalCoordinates : public ClassThatKeepsCoordinatesFromDistances
{
public:
    long distancesLeft[TRAVEL_SEGMENT_COUNT];
    long distancesRight[TRAVEL_SEGMENT_COUNT];

    void calculateForAll()
    {
        long distances[MOTOR_COUNT];
        for (size_t i = 0; i < TRAVEL_SEGMENT_COUNT; ++i) {
            distances[LEFT] = distancesLeft[i];
            distances[RIGHT] = distancesRight[i];
            calculateNewCoordinates(distances);
        }
#ifdef SIM
        std::cout << coordinates.x[LEFT] << ' ' << coordinates.y[LEFT] << std::endl;
#endif
#ifndef SIM
        Serial.print(coordinates.x[LEFT]);
        Serial.print(' ');
        Serial.println(coordinates.y[LEFT]);
#endif
    }

    void save(const size_t& index, long distances[MOTOR_COUNT]) {
        distancesLeft[index] = distances[LEFT];
        distancesRight[index] = distances[RIGHT];
    }
};

class MotorController : public ClassThatKeepsCoordinatesFromDistances
{
public:  // private

    int forwardPowerNeeded[MOTOR_COUNT];  // the power needed to travel twelve inches / number of segments
    int turnPowerNeeded[MOTOR_COUNT];

    // inherited
    // RobotCoordinates coordinates;  // relative from where I started this movement

    GlobalCoordinates global;  // for the whole grid, doesn't reset with each move

    int travelSegmentsRemaining;  // counts down

    MotorInterfaceBase* motorInterface;
    double startEncoderValues[MOTOR_COUNT];

    /** reset everything for a new movement */
    void reset()
    {
        coordinates.x[LEFT] = 0;
        coordinates.y[LEFT] = 0;
        coordinates.x[RIGHT] = WIDTH;
        coordinates.y[RIGHT] = 0;

        travelSegmentsRemaining = TRAVEL_SEGMENT_COUNT;

        global.calculateForAll();

        startEncoderValues[LEFT] = motorInterface->readEncoder(LEFT);
        startEncoderValues[RIGHT] = motorInterface->readEncoder(RIGHT);
    }

    /**
     *  pythagorean theorem - distance from where we want wheel to end
     *  if it has gone too far, it returns 0 (not negative)
     *
     *  whichMovement is LEFT, RIGHT, FORWARD
     *  whichWheel is LEFT, RIGHT
     *  howMuchXMatters is on a scale of 0 to 2 - default 1 is real pythagorean distance
     */
    double distanceFromGoal(const size_t& whichMovement, const size_t& whichWheel, const double& howMuchXMatters = 1)
    {
        double xDifference;
        double yDifference;

        if (whichMovement == FORWARD)
        {
            xDifference = (whichWheel * WIDTH) - coordinates.x[whichWheel];
            yDifference = TWELVE_INCH_DISTANCE - coordinates.y[whichWheel];
            if (yDifference < 0)  // too far
                return 0;
        }
        else  // LEFT or RIGHT
        {
            xDifference = WIDTH / 2 - coordinates.x[whichWheel];
            if (xDifference < 0 && whichWheel == LEFT)  // too far
                return 0;
            if (xDifference > 0 && whichWheel == RIGHT)  // too far
                return 0;

            if (whichMovement == LEFT)
            {
                yDifference = (WIDTH * (whichWheel - 0.5)) - coordinates.y[whichWheel];
            }
            else  // RIGHT
            {
                yDifference = (WIDTH * (0.5 - whichWheel)) - coordinates.y[whichWheel];
            }
        }

        return sqrt(howMuchXMatters * xDifference*xDifference + (2-howMuchXMatters) * yDifference*yDifference);
    }

    /**
     *  on a scale of 0 to 2, how much we care about x axis in movement
     *  movement is LEFT (0) or RIGHT (1) or anything else being forward
     */
    double howMuchToCareAboutX(const size_t& movementType)
    {
        if (movementType == LEFT || movementType == RIGHT)
        {
            // care more about y at the beginning and more about x later
            // from .33 to 1.67
            return ((TRAVEL_SEGMENT_COUNT - travelSegmentsRemaining) * 1.8 / TRAVEL_SEGMENT_COUNT) + 0.1;
        }
        // care more about x at the beginning of the move, less at the end
        // from 1.67 to 0
        return 1.6667 * travelSegmentsRemaining / TRAVEL_SEGMENT_COUNT;
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
        if (speed > (MIN_MOTOR_POWER / 2))
        {
            return MIN_MOTOR_POWER;
        }
        return 0;
    }

    int calculateProportionalCalculation(const double& progressMade,
                                         const double& progressNeededToMake,
                                         const int& gaveForThisSegment)
    {
        if (progressMade)  // > 0
        {
            return motorSpeedLimit((int) round(progressNeededToMake *
                                               gaveForThisSegment /
                                               progressMade));
        }
        else  // didn't move
        {
            // did I need to move
            if (progressNeededToMake > 20)
            {
                return MIN_MOTOR_POWER;
            }
            // else didn't need to move
            {
                // trying to move such a small distance (encoder units) isn't worth it
                // (it will probably overshoot and go too far)
                return 0;
            }
        }
    }

public:
    // constructor
    MotorController(MotorInterfaceBase* _motorInterface)
    {
        motorInterface = _motorInterface;
        reset();

        forwardPowerNeeded[LEFT] = STARTING_FORWARD_POWER_NEEDED_FOR_LEFT;
        forwardPowerNeeded[RIGHT] = STARTING_FORWARD_POWER_NEEDED_FOR_RIGHT;
        turnPowerNeeded[LEFT] = STARTING_TURN_POWER_NEEDED_FOR_LEFT;
        turnPowerNeeded[RIGHT] = STARTING_TURN_POWER_NEEDED_FOR_RIGHT;

    }

    /** movementType is FORWARD, LEFT, RIGHT */
    void go(const size_t& movementType)
    {
        reset();

        long previousEncoderReading[MOTOR_COUNT];
        long newEncoderReading[MOTOR_COUNT];
        long distancesTraveledThisTime[MOTOR_COUNT];
        double previousDistanceFromGoal[MOTOR_COUNT];
        double newDistanceFromGoal[MOTOR_COUNT];
        double progressMade[MOTOR_COUNT];
        double progressNeedToMake[MOTOR_COUNT];

        double howMuchWeCareAboutXThisTime;

        long totalPower[MOTOR_COUNT];  // excluding first and last - for taking the average
        totalPower[LEFT] = 0;
        totalPower[RIGHT] = 0;

        int* powerToGiveForThisSegment[MOTOR_COUNT];
        int direction[MOTOR_COUNT];  // 1 or -1 depending on movementType

        if (movementType == FORWARD)
        {
            powerToGiveForThisSegment[LEFT] = &forwardPowerNeeded[LEFT];
            powerToGiveForThisSegment[RIGHT] = &forwardPowerNeeded[RIGHT];
            direction[LEFT] = 1;
            direction[RIGHT] = 1;
        }
        else  // turn
        {
            powerToGiveForThisSegment[LEFT] = &turnPowerNeeded[LEFT];
            powerToGiveForThisSegment[RIGHT] = &turnPowerNeeded[RIGHT];
            if (movementType == LEFT)
            {
                direction[LEFT] = -1;
                direction[RIGHT] = 1;
            }
            else  // RIGHT
            {
                direction[LEFT] = 1;
                direction[RIGHT] = -1;
            }
        }

        while (travelSegmentsRemaining > 0)
        {
            // look for the average power over the segments - excluding first and last because they are often anomalies
            if (travelSegmentsRemaining > 1 && travelSegmentsRemaining < (TRAVEL_SEGMENT_COUNT - 1))
            {
                totalPower[LEFT] += *(powerToGiveForThisSegment[LEFT]);
                totalPower[RIGHT] += *(powerToGiveForThisSegment[RIGHT]);
            }

            howMuchWeCareAboutXThisTime = howMuchToCareAboutX(movementType);

#ifdef SIM
            std::cout << "time left: " << travelSegmentsRemaining
                      << " input left " << *(powerToGiveForThisSegment[LEFT])
                      << " right " << *(powerToGiveForThisSegment[RIGHT]) << std::endl;
            std::cout << "caring about x: " << howMuchWeCareAboutXThisTime << std::endl;
#endif
            // TODO: disable this because serial communication can affect timing
#ifndef SIM
            Serial.print("time left ");
            Serial.print(travelSegmentsRemaining);
            Serial.print(" input left ");
            Serial.print(*(powerToGiveForThisSegment[LEFT]));
            Serial.print(" right ");
            Serial.println(*(powerToGiveForThisSegment[RIGHT]));
#endif

            previousEncoderReading[LEFT] = motorInterface->readEncoder(LEFT);
            previousEncoderReading[RIGHT] = motorInterface->readEncoder(RIGHT);

            previousDistanceFromGoal[LEFT] = distanceFromGoal(movementType, LEFT, howMuchWeCareAboutXThisTime);
            previousDistanceFromGoal[RIGHT] = distanceFromGoal(movementType, RIGHT, howMuchWeCareAboutXThisTime);

            progressNeedToMake[LEFT] = previousDistanceFromGoal[LEFT] / travelSegmentsRemaining;
            progressNeedToMake[RIGHT] = previousDistanceFromGoal[RIGHT] / travelSegmentsRemaining;

            motorInterface->setMotorPower(LEFT, *(powerToGiveForThisSegment[LEFT]), direction[LEFT]);
            motorInterface->setMotorPower(RIGHT, *(powerToGiveForThisSegment[RIGHT]), direction[RIGHT]);

            motorInterface->passTime(movementType);
            --travelSegmentsRemaining;

            newEncoderReading[LEFT] = motorInterface->readEncoder(LEFT);
            newEncoderReading[RIGHT] = motorInterface->readEncoder(RIGHT);

            distancesTraveledThisTime[LEFT] = newEncoderReading[LEFT] - previousEncoderReading[LEFT];
            distancesTraveledThisTime[RIGHT] = newEncoderReading[RIGHT] - previousEncoderReading[RIGHT];

            global.save((size_t)travelSegmentsRemaining, distancesTraveledThisTime);

            calculateNewCoordinates(distancesTraveledThisTime);

            // TODO: test whether we get better results if we do this the last time
            if (travelSegmentsRemaining)  // don't do this the last time
            {
                newDistanceFromGoal[LEFT] = distanceFromGoal(movementType, LEFT, howMuchWeCareAboutXThisTime);
                newDistanceFromGoal[RIGHT] = distanceFromGoal(movementType, RIGHT, howMuchWeCareAboutXThisTime);

                progressMade[LEFT] = previousDistanceFromGoal[LEFT] - newDistanceFromGoal[LEFT];
                progressMade[RIGHT] = previousDistanceFromGoal[RIGHT] - newDistanceFromGoal[RIGHT];

                // calculate what I will need to do in the future
                int proportionalCalculation[MOTOR_COUNT];  // what I should have done this last time

                proportionalCalculation[LEFT] = calculateProportionalCalculation(progressMade[LEFT],
                                                                                 progressNeedToMake[LEFT],
                                                                                 *(powerToGiveForThisSegment[LEFT]));
                proportionalCalculation[RIGHT] = calculateProportionalCalculation(progressMade[RIGHT],
                                                                                  progressNeedToMake[RIGHT],
                                                                                  *(powerToGiveForThisSegment[RIGHT]));

                // now set new values to give next segment
                *(powerToGiveForThisSegment[LEFT]) = proportionalCalculation[LEFT];
                *(powerToGiveForThisSegment[RIGHT]) = proportionalCalculation[RIGHT];

            }

#ifdef SIM
            std::cout << motorInterface->report() << std::endl;

#endif
        }

        motorInterface->setMotorPower(LEFT, 0, 1);
        motorInterface->setMotorPower(RIGHT, 0, 1);

        // calculate average power
        double average[MOTOR_COUNT];
        average[LEFT] = (double)(totalPower[LEFT]) / (TRAVEL_SEGMENT_COUNT - 2);  // minus first and last
        average[RIGHT] = (double)(totalPower[RIGHT]) / (TRAVEL_SEGMENT_COUNT - 2);
#ifdef SIM
        std::cout << "average left power: " << average[LEFT] << std::endl;
        std::cout << "average right power: " << average[RIGHT] << std::endl;
#endif
#ifndef SIM
        Serial.print("average left power: ");
        Serial.println(average[LEFT]);
        Serial.print("average right power: ");
        Serial.println(average[RIGHT]);
#endif
    }
};

#endif //ALL_FUNC_WHEELS_MOTORCONTROLLER_H
