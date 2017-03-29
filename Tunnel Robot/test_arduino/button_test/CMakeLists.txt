cmake_minimum_required(VERSION 3.6)
project(button_test)

set(CMAKE_CXX_STANDARD 11)

include_directories(
        F:/Arduino/hardware/arduino/avr/cores/arduino
        /usr/share/arduino/hardware/arduino/cores/arduino
)

set(SOURCE_FILES
        sketch/sketch.ino)
add_executable(button_test ${SOURCE_FILES})