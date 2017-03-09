#ifndef ARDUINO_H_INCLUDED
#define ARDUINO_H_INCLUDED

#include <iostream>
#include <string>

#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define INPUT 0

typedef uint8_t byte;
typedef bool boolean;
typedef std::string String;

class Serial_Class {
public:
    void print(const std::string& param) {
        std::cout << param;
    }

    void println(const std::string& param) {
        std::cout << param << std::endl;
    }

    void println(const long int& param) {
        std::cout << param << std::endl;
    }

    void begin(const int& a) {}
};

Serial_Class Serial;

long millis() { return 777; }
int digitalRead(const int& pin) { return pin; }
int pinMode(const int& pin, const int& mode) { return mode; }
int clearDigit() { return 1; }

#endif // ARDUINO_H_INCLUDED
