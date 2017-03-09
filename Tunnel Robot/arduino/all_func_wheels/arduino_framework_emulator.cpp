#include <iostream>

// the main sketch.ino file
#include "src/sketch/sketch.ino.cpp"

int main()
{
    setup();
    loop();
    std::cout << "complete\n";
    return 0;
}
