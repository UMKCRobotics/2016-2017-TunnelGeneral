#ifndef ADAFRUIT_NEOMATRIX_H_INCLUDED
#define ADAFRUIT_NEOMATRIX_H_INCLUDED

#define NEO_MATRIX_TOP 1
#define NEO_MATRIX_RIGHT 1
#define NEO_MATRIX_COLUMNS 8
#define NEO_MATRIX_PROGRESSIVE 1
#define NEO_GRB 1
#define NEO_KHZ800 1

class Adafruit_NeoMatrix {

public:
    Adafruit_NeoMatrix(int a, int b, int c, int d, int e) {}

    void begin() {}
    void setBrightness(const int& a) {}
    void show() {}
};

#endif // ADAFRUIT_NEOMATRIX_H_INCLUDED
