// keep cartesian coordinates using geometry

#ifndef ALL_FUNC_WHEELS_CLASSTHATKEEPSCOORDINATESFROMDISTANCES_H
#define ALL_FUNC_WHEELS_CLASSTHATKEEPSCOORDINATESFROMDISTANCES_H

#include "MotorInterfaceBase.h"

#define CARTESIAN_STRATEGY  // used in OurRobotSpecs.h
#include "OurRobotSpecs.h"

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

#endif //ALL_FUNC_WHEELS_CLASSTHATKEEPSCOORDINATESFROMDISTANCES_H
