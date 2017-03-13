#ifndef PIDNOTIME_H
#define PIDNOTIME_H

// #include "Arduino.h"
// for arduino comment out from here down to including millis
#include <iostream>
#include <chrono>

class SerialClass
{
public:
    static void print(const char* str)
    {
        std::cout << str;
    }
    static void println(const long& a)
    {
        std::cout << a;
    }
};
SerialClass Serial;

typedef long double bigfloat;

template<class T>class PID{
private:

    bigfloat integral;
    bigfloat derivative;
    T prev_error;
    bigfloat integral_gain;
    bigfloat derivative_gain;
    bigfloat proportional_gain;
    long prev_output;
public:
    void logDiagnosticInfo(){
        Serial.print("output:" );
        Serial.println(prev_output);
        Serial.print("error: ");
        Serial.println(prev_error);
    }

    void reset()
    {
        integral = 0;
        prev_error = 0;
    }

    PID(){
        setGains(1,1,1);
        reset();
    }

    PID(bigfloat p_gain, bigfloat i_gain, bigfloat d_gain){
        setGains(p_gain,i_gain,d_gain);
        reset();
    }

    void setGains(bigfloat p_gain, bigfloat i_gain, bigfloat d_gain){
        proportional_gain = p_gain;
        integral_gain = i_gain;
        derivative_gain = d_gain;

    }

    //Pretty much taken directly from https://en.wikipedia.org/wiki/PID_controller#Pseudocode
    T calculate(T measured_value, T set_point){

        T error = set_point - measured_value;
        integral = integral + error;
        derivative = error - prev_error;
        T output = proportional_gain * error + integral_gain * integral + derivative_gain * derivative;
        prev_error = error;

        prev_output = output;
        return output;
    }
};

#endif //PIDNOTIME_H
