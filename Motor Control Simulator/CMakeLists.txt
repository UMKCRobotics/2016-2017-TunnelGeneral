cmake_minimum_required(VERSION 3.6)
project(Motor_Control_Simulator)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        main.cpp PIDnoTime.h)

add_executable(Motor_Control_Simulator ${SOURCE_FILES})