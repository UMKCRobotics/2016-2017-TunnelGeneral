#ifndef ARDUINO_H_INCLUDED
#define ARDUINO_H_INCLUDED

#include <iostream>
#include <string>

#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define INPUT 0
#define CHANGE 1

typedef uint8_t byte;
typedef bool boolean;

class String {
public:
    std::string data;

    String() {

    }

    String(const char* c_str) {
        data = c_str;
    }

    String(const int& a) {
        data += "7";  // TODO: convert int a
    }

    String operator= (const char* c_str) {
        data = c_str;

        return *this;
    }

    String operator= (const std::string param) {
        data = param;

        return *this;
    }

    String operator+= (const char a) {
        data += a;

        return *this;
    }

    String operator+= (const String& a) {
        data += a.data;

        return *this;
    }

    bool operator== (const char* rhs) {
        return data == rhs;
    }

    int toInt() { return 1; }  // TODO: real conversion
};

class Serial_Class {
public:
    void print(const std::string& param) {
        std::cout << param;
    }

    void write(const char& param) {
        std::cout << param;
    }

    void println(const std::string& param) {
        std::cout << param << std::endl;
    }

    void println(const String& param) {
        std::cout << param.data << std::endl;
    }

    void println(const long int& param) {
        std::cout << param << std::endl;
    }

    char read() { return '\n'; }

    void begin(const int& a) {}

    bool available() { return true; }
};

Serial_Class Serial;

long millis() { return 777; }
int analogWrite(int a, int b) {return b;}
int analogRead(int pin) { return pin; }
int digitalWrite(int a, int b) {return b;}
int digitalRead(const int& pin) { return pin; }
int digitalPinToInterrupt(const int& pin) { return pin; };
void attachInterrupt(int a, void (*)(), int c) {};
int pinMode(const int& pin, const int& mode) { return mode; }
int clearDigit() { return 1; }
int delay(const int& a) { return a; }

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#endif // ARDUINO_H_INCLUDED
