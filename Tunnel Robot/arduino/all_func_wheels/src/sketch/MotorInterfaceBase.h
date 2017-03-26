
#ifndef ALL_FUNC_WHEELS_MOTORINTERFACEBASE_H
#define ALL_FUNC_WHEELS_MOTORINTERFACEBASE_H

#ifdef SIM
    #include <cstddef>  // size_t
#endif
#ifndef SIM
    #include "Arduino.h"  // size_t
#endif

// indexes for arrays
#define LEFT 0
#define RIGHT 1
#define MOTOR_COUNT 2

class MotorInterfaceBase
{
public:
    virtual const long readEncoder(const size_t& which) const = 0;
    virtual void setMotorPower(const size_t& which, const int& howMuch, const int& direction) = 0;
    virtual const char* report() const = 0;
};

#endif //ALL_FUNC_WHEELS_MOTORINTERFACEBASE_H
