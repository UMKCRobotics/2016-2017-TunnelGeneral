#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

// indexes for arrays
#define LEFT 0
#define RIGHT 1
#define MOTOR_COUNT 2

#define WIDTH 500  // distance from left wheel to right wheel - in units that the encoder gives me

class MotorInterface
{
public:  // would be private in a different setting
    double distanceTraveled[MOTOR_COUNT];
    double totalMeasuredAboveTheOther[MOTOR_COUNT];  // TODO: different class
    int currentPower[MOTOR_COUNT];
    double cartesianCoordinateX[MOTOR_COUNT];
    double cartesianCoordinateY[MOTOR_COUNT];

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
        double uturXleft;
        double uturYleft;

        double uturXright;
        double uturYright;

        // utr = untranslated rotated
        double utrXleft;
        double utrYleft;

        double utrXright;
        double utrYright;

        // angle of the distance around a circle that the robot traveled (radians)
        double t = (distance[LEFT] - distance[RIGHT]) / WIDTH;

        double cos_t = std::cos(t);
        double sin_t = std::sin(t);

        // distance from the center of the circle to the right wheel
        double r;
        if (t)  // != 0
        {
            r = distance[RIGHT] / t;

            uturXleft = (r+WIDTH) * (1 - cos_t);
            uturYleft = (r+WIDTH) * sin_t;

            uturXright = (r + WIDTH) - (r * cos_t);
            uturYright = r * sin_t;
        }
        else  // robot went straight
        {
            uturXleft = 0;
            uturYleft = distance[LEFT];

            uturXright = WIDTH;
            uturYright = distance[RIGHT];
        }

        // rotate and translate onto current coordinates

        // first rotate - by the angle of the right wheel
        // we want to undo the rotation that would bring the current corrdinates back to straight,
        // so we use the clockwise (backwards) rotation matrix
        t = std::atan2(cartesianCoordinateY[LEFT] - cartesianCoordinateY[RIGHT],
                       cartesianCoordinateX[RIGHT] - cartesianCoordinateX[LEFT]);
        cos_t = std::cos(t);
        sin_t = std::sin(t);

        utrXleft = uturXleft * cos_t + uturYleft * sin_t;
        utrYleft = uturYleft * cos_t - uturXleft * sin_t;

        utrXright = uturXright * cos_t + uturYright * sin_t;
        utrYright = uturYright * cos_t - uturXright * sin_t;

        // now translate - add left wheel coordinate
        // right first, so we don't lose the left wheel coordinate
        cartesianCoordinateX[RIGHT] = utrXright + cartesianCoordinateX[LEFT];
        cartesianCoordinateY[RIGHT] = utrYright + cartesianCoordinateY[LEFT];

        cartesianCoordinateX[LEFT] += utrXleft;
        cartesianCoordinateY[LEFT] += utrYleft;
    }

public:
    /** default constructor */
    MotorInterface()
    {
        for (size_t i = 0; i < MOTOR_COUNT; ++i)
        {
            distanceTraveled[i] = 0;
            currentPower[i] = 0;
            cartesianCoordinateX[i] = i * WIDTH;
            cartesianCoordinateY[i] = 0;
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
        return cartesianCoordinateX[wheel];
    }
    const double& getYofWheel(const size_t& wheel) const
    {
        return cartesianCoordinateY[wheel];
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
