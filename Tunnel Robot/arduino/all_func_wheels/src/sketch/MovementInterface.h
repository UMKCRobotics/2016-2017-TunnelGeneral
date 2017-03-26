// interface for moving our robot
// (more abstract than MotorInterface)

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

#include "MotorInterfaceBase.h"
#include "ClassThatKeepsCoordinatesFromDistances.h"
#ifndef SIM
    #include "MotorInterface.h"
#endif

#define FORWARD 2

#define TRAVEL_SEGMENT_COUNT 10  // the number of segments to break the travel into

#define FORWARD_TRAVEL_DURATION 3000  // milliseconds for one grid move
const int FORWARD_SEGMENT_DURATION = FORWARD_TRAVEL_DURATION / TRAVEL_SEGMENT_COUNT;

#define TURN_TRAVEL_DURATION 1500  // milliseconds to turn
const int TURN_SEGMENT_DURATION = TURN_TRAVEL_DURATION / TRAVEL_SEGMENT_COUNT;

// TODO: tune with average values in log
// one motor consistently weaker than the other? make its number higher here
// TODO: these should also be a function of duration - I'm tired of math
const double STARTING_FORWARD_POWER_PER_DISTANCE_NEEDED_FOR_LEFT
        = 1.36;
const double STARTING_FORWARD_POWER_PER_DISTANCE_NEEDED_FOR_RIGHT
        = 1.36;
const double STARTING_TURN_POWER_PER_DISTANCE_NEEDED_FOR_LEFT
        = 1.8;  // absolute values
const double STARTING_TURN_POWER_PER_DISTANCE_NEEDED_FOR_RIGHT
        = 1.8;

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
#ifdef VERBOSE
        Serial.print(coordinates.x[LEFT]);
        Serial.print(' ');
        Serial.println(coordinates.y[LEFT]);
#endif  // VERBOSE
#endif  // not SIM
    }

    void save(const size_t& index, long distances[MOTOR_COUNT]) {
        distancesLeft[index] = distances[LEFT];
        distancesRight[index] = distances[RIGHT];
    }
};

class MovementInterface : public ClassThatKeepsCoordinatesFromDistances
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
    MovementInterface(MotorInterfaceBase* _motorInterface)
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
            *(powerPerDistanceForThisMovement[LEFT]) = STARTING_FORWARD_POWER_PER_DISTANCE_NEEDED_FOR_LEFT;
            *(powerPerDistanceForThisMovement[RIGHT]) = STARTING_FORWARD_POWER_PER_DISTANCE_NEEDED_FOR_RIGHT;
            direction[LEFT] = 1;
            direction[RIGHT] = 1;
        }
        else  // turn
        {
            powerPerDistanceForThisMovement[LEFT] = &turnPowerPerDistance[LEFT];
            powerPerDistanceForThisMovement[RIGHT] = &turnPowerPerDistance[RIGHT];
            *(powerPerDistanceForThisMovement[LEFT]) = STARTING_TURN_POWER_PER_DISTANCE_NEEDED_FOR_LEFT;
            *(powerPerDistanceForThisMovement[RIGHT]) = STARTING_TURN_POWER_PER_DISTANCE_NEEDED_FOR_RIGHT;
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
#ifdef VERBOSE
            Serial.print("time left ");
            Serial.println(travelSegmentsRemaining);
            /*
            Serial.print(" input left ");
            Serial.print(*(powerPerDistanceForThisMovement[LEFT]));
            Serial.print(" right ");
            Serial.println(*(powerPerDistanceForThisMovement[RIGHT]));
             */
#endif  // VERBOSE
#endif  // not SIM

            previousDistanceFromGoal[LEFT] = distanceFromGoal(movementType, LEFT, howMuchWeCareAboutXThisTime);
            previousDistanceFromGoal[RIGHT] = distanceFromGoal(movementType, RIGHT, howMuchWeCareAboutXThisTime);

            /*Serial.print("I see distance from my goal ");
            Serial.print(previousDistanceFromGoal[LEFT]);
            Serial.print(' ');
            Serial.println(previousDistanceFromGoal[RIGHT]);*/

            // distance I need to go this segment
            progressNeedToMake[LEFT] = previousDistanceFromGoal[LEFT] / travelSegmentsRemaining;
            progressNeedToMake[RIGHT] = previousDistanceFromGoal[RIGHT] / travelSegmentsRemaining;

            /*Serial.print("In this segment I want to go ");
            Serial.print(progressNeedToMake[LEFT]);
            Serial.print(' ');
            Serial.println(progressNeedToMake[RIGHT]);*/
            
            powerToGive[LEFT] = motorSpeedLimit((int)round(progressNeedToMake[LEFT] *
                                                           *(powerPerDistanceForThisMovement[LEFT])));
            powerToGive[RIGHT] = motorSpeedLimit((int)round(progressNeedToMake[RIGHT] *
                                                            *(powerPerDistanceForThisMovement[RIGHT])));

#ifdef VERBOSE
            Serial.print("giving the motors power: ");
            Serial.print(powerToGive[LEFT]);
            Serial.print(' ');
            Serial.println(powerToGive[RIGHT]);
#endif

            motorInterface->setMotorPower(LEFT, powerToGive[LEFT], direction[LEFT]);
            motorInterface->setMotorPower(RIGHT, powerToGive[RIGHT], direction[RIGHT]);

            passTime(movementType);
            --travelSegmentsRemaining;

            newEncoderReading[LEFT] = motorInterface->readEncoder(LEFT);
            newEncoderReading[RIGHT] = motorInterface->readEncoder(RIGHT);

            distancesTraveledThisTime[LEFT] = newEncoderReading[LEFT] - previousEncoderReading[LEFT];
            distancesTraveledThisTime[RIGHT] = newEncoderReading[RIGHT] - previousEncoderReading[RIGHT];

            /*Serial.print("This is the distance each wheel traveled ");
            Serial.print(distancesTraveledThisTime[LEFT]);
            Serial.print(' ');
            Serial.println(distancesTraveledThisTime[RIGHT]);*/
            
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
#ifdef VERBOSE
        Serial.print("average left power: ");
        Serial.println(average[LEFT]);
        Serial.print("average right power: ");
        Serial.println(average[RIGHT]);
#endif  // VERBOSE
#endif  // not SIM
    }

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
     * @param direction 1 or -1
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

#endif //ALL_FUNC_WHEELS_MOTORCONTROLLER_H
