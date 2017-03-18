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

// TODO: these might be able to be tuned better
// WIDTH tuned by making turns accurate 90 degrees
// TWELVE_INCH_DISTANCE tuned by making forward accurate 12 inches
#define WIDTH 1617  // distance from left wheel to right wheel - in units that the encoder gives me
#define TWELVE_INCH_DISTANCE 2427  // in units of the encoder

#define TRAVEL_SEGMENT_COUNT 30  // the number of segments to break the travel into

#define FORWARD_TRAVEL_DURATION 3000  // milliseconds for one grid move
const int FORWARD_SEGMENT_DURATION = FORWARD_TRAVEL_DURATION / TRAVEL_SEGMENT_COUNT;

#define TURN_TRAVEL_DURATION 2000  // milliseconds to turn
const int TURN_SEGMENT_DURATION = TURN_TRAVEL_DURATION / TRAVEL_SEGMENT_COUNT;

// TODO: tune with average values in log
// one motor consistently weaker than the other? make its number higher here
// TODO: these should also be a function of duration - I'm tired of math
const double STARTING_FORWARD_POWER_PER_DISTANCE_NEEDED_FOR_LEFT
        = 150.0 / (TWELVE_INCH_DISTANCE / TRAVEL_SEGMENT_COUNT);
const double STARTING_FORWARD_POWER_PER_DISTANCE_NEEDED_FOR_RIGHT
        = 150.0 / (TWELVE_INCH_DISTANCE / TRAVEL_SEGMENT_COUNT);
const double STARTING_TURN_POWER_PER_DISTANCE_NEEDED_FOR_LEFT
        = STARTING_FORWARD_POWER_PER_DISTANCE_NEEDED_FOR_LEFT * 4 / 3;  // absolute values
const double STARTING_TURN_POWER_PER_DISTANCE_NEEDED_FOR_RIGHT
        = STARTING_FORWARD_POWER_PER_DISTANCE_NEEDED_FOR_RIGHT * 4 / 3;

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

    double forwardPowerPerDistance[MOTOR_COUNT];  // the power needed to travel twelve inches / number of segments
    double turnPowerPerDistance[MOTOR_COUNT];

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
        return MIN_MOTOR_POWER;
    }

    /**
     *  power per distance that I found I make
     *  with exponential weighted average
     */
    double proportionalPowerPerDistance(const double &progressMade,
                                        const int &powerGaveForThisSegment,
                                        const double &previousPowerPerDistance)
    {
        if (progressMade)  // > 0
        {
            return ((powerGaveForThisSegment / progressMade) + previousPowerPerDistance) / 2;
        }
        else  // didn't move
        {
            return previousPowerPerDistance;
        }
    }

public:
    // constructor
    MotorController(MotorInterfaceBase* _motorInterface)
    {
        motorInterface = _motorInterface;
        reset();

        forwardPowerPerDistance[LEFT] = STARTING_FORWARD_POWER_PER_DISTANCE_NEEDED_FOR_LEFT;
        forwardPowerPerDistance[RIGHT] = STARTING_FORWARD_POWER_PER_DISTANCE_NEEDED_FOR_RIGHT;
        turnPowerPerDistance[LEFT] = STARTING_TURN_POWER_PER_DISTANCE_NEEDED_FOR_LEFT;
        turnPowerPerDistance[RIGHT] = STARTING_TURN_POWER_PER_DISTANCE_NEEDED_FOR_RIGHT;

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
        int powerToGive[MOTOR_COUNT];

        double howMuchWeCareAboutXThisTime;

        long totalPower[MOTOR_COUNT];  // excluding first and last - for taking the average
        totalPower[LEFT] = 0;
        totalPower[RIGHT] = 0;

        double* powerPerDistanceForThisMovement[MOTOR_COUNT];
        int direction[MOTOR_COUNT];  // 1 or -1 depending on movementType

        if (movementType == FORWARD)
        {
            powerPerDistanceForThisMovement[LEFT] = &forwardPowerPerDistance[LEFT];
            powerPerDistanceForThisMovement[RIGHT] = &forwardPowerPerDistance[RIGHT];
            direction[LEFT] = 1;
            direction[RIGHT] = 1;
        }
        else  // turn
        {
            powerPerDistanceForThisMovement[LEFT] = &turnPowerPerDistance[LEFT];
            powerPerDistanceForThisMovement[RIGHT] = &turnPowerPerDistance[RIGHT];
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

        // get encoder readings before we start moving
        previousEncoderReading[LEFT] = motorInterface->readEncoder(LEFT);
        previousEncoderReading[RIGHT] = motorInterface->readEncoder(RIGHT);

        while (travelSegmentsRemaining > 0)
        {
            // look for the average power over the segments - excluding first and last because they are often anomalies
            if (travelSegmentsRemaining > 1 && travelSegmentsRemaining < (TRAVEL_SEGMENT_COUNT - 1))
            {
                totalPower[LEFT] += *(powerPerDistanceForThisMovement[LEFT]);
                totalPower[RIGHT] += *(powerPerDistanceForThisMovement[RIGHT]);
            }

            howMuchWeCareAboutXThisTime = howMuchToCareAboutX(movementType);

#ifdef SIM
            std::cout << "time left: " << travelSegmentsRemaining
                      << " input left " << *(powerPerDistanceForThisMovement[LEFT])
                      << " right " << *(powerPerDistanceForThisMovement[RIGHT]) << std::endl;
            std::cout << "caring about x: " << howMuchWeCareAboutXThisTime << std::endl;
#endif
            // TODO: disable this because serial communication can affect timing
            // TODO: don't remove if we haven't done a lot of testing without it
#ifndef SIM
            
            Serial.print("time left ");
            Serial.println(travelSegmentsRemaining);
            /*
            Serial.print(" input left ");
            Serial.print(*(powerPerDistanceForThisMovement[LEFT]));
            Serial.print(" right ");
            Serial.println(*(powerPerDistanceForThisMovement[RIGHT]));
             */
#endif

            previousDistanceFromGoal[LEFT] = distanceFromGoal(movementType, LEFT, howMuchWeCareAboutXThisTime);
            previousDistanceFromGoal[RIGHT] = distanceFromGoal(movementType, RIGHT, howMuchWeCareAboutXThisTime);

            Serial.print("I see distance from my goal ");
            Serial.print(previousDistanceFromGoal[LEFT]);
            Serial.print(' ');
            Serial.println(previousDistanceFromGoal[RIGHT]);

            // distance I need to go this segment
            progressNeedToMake[LEFT] = previousDistanceFromGoal[LEFT] / travelSegmentsRemaining;
            progressNeedToMake[RIGHT] = previousDistanceFromGoal[RIGHT] / travelSegmentsRemaining;

            Serial.print("In this segment I want to go ");
            Serial.print(progressNeedToMake[LEFT]);
            Serial.print(' ');
            Serial.println(progressNeedToMake[RIGHT]);
            
            powerToGive[LEFT] = motorSpeedLimit((int)round(progressNeedToMake[LEFT] *
                                                           *(powerPerDistanceForThisMovement[LEFT])));
            powerToGive[RIGHT] = motorSpeedLimit((int)round(progressNeedToMake[RIGHT] *
                                                            *(powerPerDistanceForThisMovement[RIGHT])));

            Serial.print("So I'm going to give the motor this much power ");
            Serial.print(powerToGive[LEFT]);
            Serial.print(' ');
            Serial.println(powerToGive[RIGHT]);

            motorInterface->setMotorPower(LEFT, powerToGive[LEFT], direction[LEFT]);
            motorInterface->setMotorPower(RIGHT, powerToGive[RIGHT], direction[RIGHT]);

            motorInterface->passTime(movementType);
            --travelSegmentsRemaining;

            newEncoderReading[LEFT] = motorInterface->readEncoder(LEFT);
            newEncoderReading[RIGHT] = motorInterface->readEncoder(RIGHT);

            distancesTraveledThisTime[LEFT] = newEncoderReading[LEFT] - previousEncoderReading[LEFT];
            distancesTraveledThisTime[RIGHT] = newEncoderReading[RIGHT] - previousEncoderReading[RIGHT];

            Serial.print("This is the distance each wheel traveled ");
            Serial.print(distancesTraveledThisTime[LEFT]);
            Serial.print(' ');
            Serial.println(distancesTraveledThisTime[RIGHT]);
            
            // next time's previousEncoderReading is this time's newEncoderReading
            previousEncoderReading[LEFT] = newEncoderReading[LEFT];
            previousEncoderReading[RIGHT] = newEncoderReading[RIGHT];

            global.save((size_t)travelSegmentsRemaining, distancesTraveledThisTime);

            calculateNewCoordinates(distancesTraveledThisTime);

            // TODO: test whether we get better results if we do this the last time
            if (travelSegmentsRemaining)  // don't do this the last time
            {
                newDistanceFromGoal[LEFT] = distanceFromGoal(movementType, LEFT, howMuchWeCareAboutXThisTime);
                newDistanceFromGoal[RIGHT] = distanceFromGoal(movementType, RIGHT, howMuchWeCareAboutXThisTime);

                progressMade[LEFT] = previousDistanceFromGoal[LEFT] - newDistanceFromGoal[LEFT];
                progressMade[RIGHT] = previousDistanceFromGoal[RIGHT] - newDistanceFromGoal[RIGHT];

                /*
                double previousPowerPerDistance[MOTOR_COUNT];  // power per distance used this last time

                previousPowerPerDistance[LEFT] = *(powerPerDistanceForThisMovement[LEFT]);
                previousPowerPerDistance[RIGHT] = *(powerPerDistanceForThisMovement[RIGHT]);
                 */

                // update power per distance with information gathered
                if (travelSegmentsRemaining != TRAVEL_SEGMENT_COUNT - 1)  // not first time because motors take time to start
                {
                    *(powerPerDistanceForThisMovement[LEFT])
                            = proportionalPowerPerDistance(progressMade[LEFT],
                                                           powerToGive[LEFT],
                                                           *(powerPerDistanceForThisMovement[LEFT]));
                    *(powerPerDistanceForThisMovement[RIGHT])
                            = proportionalPowerPerDistance(progressMade[RIGHT],
                                                           powerToGive[RIGHT],
                                                           *(powerPerDistanceForThisMovement[RIGHT]));
                }
                // TODO: try different weighted averaging schemes
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

/* log
 � � 0.00
10.00 0.00
time left 30
I see distance from my goal 1143.36 1143.43
In this segment I want to go 38.11 38.11
So I'm going to give the motor this much power 90 90
This is the distance each wheel traveled -3 1
time left 29
I see distance from my goal 1139.43 1142.15
In this segment I want to go 39.29 39.38
So I'm going to give the motor this much power 254 254
This is the distance each wheel traveled -26 26
time left 28
I see distance from my goal 1106.81 1109.48
In this segment I want to go 39.53 39.62
So I'm going to give the motor this much power 254 254
This is the distance each wheel traveled -71 73
time left 27
I see distance from my goal 1021.79 1021.99
In this segment I want to go 37.84 37.85
So I'm going to give the motor this much power 108 105
This is the distance each wheel traveled -82 81
time left 26
I see distance from my goal 930.43 931.83
In this segment I want to go 35.79 35.84
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled -60 56
time left 25
I see distance from my goal 870.92 876.74
In this segment I want to go 34.84 35.07
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled -31 27
time left 24
I see distance from my goal 847.11 857.00
In this segment I want to go 35.30 35.71
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled -6 5
time left 23
I see distance from my goal 851.11 861.74
In this segment I want to go 37.00 37.47
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 22
I see distance from my goal 861.36 871.70
In this segment I want to go 39.15 39.62
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 21
I see distance from my goal 871.49 881.54
In this segment I want to go 41.50 41.98
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 20
I see distance from my goal 881.50 891.28
In this segment I want to go 44.08 44.56
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 19
I see distance from my goal 891.40 900.91
In this segment I want to go 46.92 47.42
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 18
I see distance from my goal 901.19 910.44
In this segment I want to go 50.07 50.58
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 17
I see distance from my goal 910.88 919.87
In this segment I want to go 53.58 54.11
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 16
I see distance from my goal 920.46 929.20
In this segment I want to go 57.53 58.08
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 15
I see distance from my goal 929.95 938.45
In this segment I want to go 62.00 62.56
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 14
I see distance from my goal 939.34 947.60
In this segment I want to go 67.10 67.69
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 13
I see distance from my goal 948.64 956.66
In this segment I want to go 72.97 73.59
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 12
I see distance from my goal 957.84 965.64
In this segment I want to go 79.82 80.47
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 11
I see distance from my goal 966.96 974.54
In this segment I want to go 87.91 88.59
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 10
I see distance from my goal 976.00 983.36
In this segment I want to go 97.60 98.34
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 9
I see distance from my goal 984.95 992.09
In this segment I want to go 109.44 110.23
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 8
I see distance from my goal 993.82 1000.76
In this segment I want to go 124.23 125.09
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 7
I see distance from my goal 1002.61 1009.34
In this segment I want to go 143.23 144.19
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 6
I see distance from my goal 1011.33 1017.86
In this segment I want to go 168.55 169.64
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 5
I see distance from my goal 1019.97 1026.30
In this segment I want to go 203.99 205.26
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 4
I see distance from my goal 1028.54 1034.68
In this segment I want to go 257.13 258.67
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 3
I see distance from my goal 1037.03 1042.98
In this segment I want to go 345.68 347.66
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 2
I see distance from my goal 1045.46 1051.23
In this segment I want to go 522.73 525.61
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 1
I see distance from my goal 1053.82 1059.41
In this segment I want to go 1053.82 1059.41
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
average left power: 0.36
average right power: 0.36
  11
46.35 -273.75
time left 30
I see distance from my goal 1143.36 1143.43
In this segment I want to go 38.11 38.11
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 29
I see distance from my goal 1143.34 1143.45
In this segment I want to go 39.43 39.43
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 28
I see distance from my goal 1143.31 1143.47
In this segment I want to go 40.83 40.84
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 27
I see distance from my goal 1143.29 1143.49
In this segment I want to go 42.34 42.35
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 26
I see distance from my goal 1143.27 1143.51
In this segment I want to go 43.97 43.98
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 25
I see distance from my goal 1143.25 1143.53
In this segment I want to go 45.73 45.74
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 24
I see distance from my goal 1143.23 1143.55
In this segment I want to go 47.63 47.65
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 23
I see distance from my goal 1143.21 1143.58
In this segment I want to go 49.70 49.72
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 22
I see distance from my goal 1143.19 1143.60
In this segment I want to go 51.96 51.98
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 21
I see distance from my goal 1143.17 1143.62
In this segment I want to go 54.44 54.46
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 20
I see distance from my goal 1143.14 1143.64
In this segment I want to go 57.16 57.18
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 19
I see distance from my goal 1143.12 1143.66
In this segment I want to go 60.16 60.19
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 18
I see distance from my goal 1143.10 1143.68
In this segment I want to go 63.51 63.54
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 17
I see distance from my goal 1143.08 1143.70
In this segment I want to go 67.24 67.28
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 16
I see distance from my goal 1143.06 1143.72
In this segment I want to go 71.44 71.48
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 15
I see distance from my goal 1143.04 1143.75
In this segment I want to go 76.20 76.25
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 14
I see distance from my goal 1143.02 1143.77
In this segment I want to go 81.64 81.70
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 13
I see distance from my goal 1143.00 1143.79
In this segment I want to go 87.92 87.98
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 12
I see distance from my goal 1142.97 1143.81
In this segment I want to go 95.25 95.32
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 11
I see distance from my goal 1142.95 1143.83
In this segment I want to go 103.90 103.98
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 10
I see distance from my goal 1142.93 1143.85
In this segment I want to go 114.29 114.39
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 9
I see distance from my goal 1142.91 1143.87
In this segment I want to go 126.99 127.10
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 8
I see distance from my goal 1142.89 1143.89
In this segment I want to go 142.86 142.99
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 7
I see distance from my goal 1142.87 1143.92
In this segment I want to go 163.27 163.42
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 6
I see distance from my goal 1142.85 1143.94
In this segment I want to go 190.47 190.66
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 5
I see distance from my goal 1142.83 1143.96
In this segment I want to go 228.57 228.79
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 4
I see distance from my goal 1142.80 1143.98
In this segment I want to go 285.70 285.99
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 3
I see distance from my goal 1142.78 1144.00
In this segment I want to go 380.93 381.33
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 2
I see distance from my goal 1142.76 1144.02
In this segment I want to go 571.38 572.01
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
time left 1
I see distance from my goal 1142.74 1144.04
In this segment I want to go 1142.74 1144.04
So I'm going to give the motor this much power 0 0
This is the distance each wheel traveled 0 0
average left power: 0.00
average right power: 0.00
  11

 */
