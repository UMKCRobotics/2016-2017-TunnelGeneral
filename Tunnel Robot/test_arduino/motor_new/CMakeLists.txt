cmake_minimum_required(VERSION 3.6)
project(motor_new)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        src/sketch/sketch.ino)

add_executable(motor_new ${SOURCE_FILES})