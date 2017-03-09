#include <iostream>

#include "Arduino.h"

// arduino compiler lets you call functions that are not declared yet
#include "predeclarations.h"

// the main sketch.ino file
#include "src/sketch/sketch.ino.cpp"

int main()
{
    setup();
    loop();
    std::cout << "complete\n";
    return 0;
}
