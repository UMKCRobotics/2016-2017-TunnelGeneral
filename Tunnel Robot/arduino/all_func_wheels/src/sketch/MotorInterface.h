// The direct interface to our specific motors

#ifndef ALL_FUNC_WHEELS_MOTORINTERFACE_H_H
#define ALL_FUNC_WHEELS_MOTORINTERFACE_H_H

#include "MotorInterfaceBase.h"
#include "Arduino.h"

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

    const char* report() const
    {
        return "";
    }
};

#endif //ALL_FUNC_WHEELS_MOTORINTERFACE_H_H
