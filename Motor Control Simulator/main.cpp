#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

// indexes for arrays
#define LEFT 0
#define RIGHT 1
#define MOTOR_COUNT 2

#define WIDTH 500  // distance from left wheel to right wheel - in units that the encoder gives me

struct RobotCoordinates
{
    double x[MOTOR_COUNT];
    double y[MOTOR_COUNT];
};

class MotorInterface
{
public:  // would be private in a different setting
    double distanceTraveled[MOTOR_COUNT];
    double totalMeasuredAboveTheOther[MOTOR_COUNT];  // TODO: different class
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

void test()
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

int main()
{
    test();

    return 0;
}
