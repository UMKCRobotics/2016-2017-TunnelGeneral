cmake_minimum_required(VERSION 3.6)
project(all_func_wheels)

set(CMAKE_CXX_STANDARD 11)

include_directories(
        lib/Adafruit-GFX-Library
        lib/Adafruit_NeoMatrix
        lib/Adafruit_NeoPixel
        F:/Arduino/hardware/arduino/avr/cores/arduino
        /usr/share/arduino/hardware/arduino/cores/arduino
)

set(SOURCE_FILES
        lib/Adafruit-GFX-Library/fontconvert/fontconvert.c
        lib/Adafruit-GFX-Library/Fonts/FreeMono12pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeMono18pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeMono24pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeMono9pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeMonoBold12pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeMonoBold18pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeMonoBold24pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeMonoBold9pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeMonoBoldOblique12pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeMonoBoldOblique18pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeMonoBoldOblique24pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeMonoBoldOblique9pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeMonoOblique12pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeMonoOblique18pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeMonoOblique24pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeMonoOblique9pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSans12pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSans18pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSans24pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSans9pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSansBold12pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSansBold18pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSansBold24pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSansBold9pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSansBoldOblique12pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSansBoldOblique18pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSansBoldOblique24pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSansBoldOblique9pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSansOblique12pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSansOblique18pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSansOblique24pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSansOblique9pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSerif12pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSerif18pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSerif24pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSerif9pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSerifBold12pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSerifBold18pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSerifBold24pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSerifBold9pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSerifBoldItalic12pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSerifBoldItalic18pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSerifBoldItalic24pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSerifBoldItalic9pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSerifItalic12pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSerifItalic18pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSerifItalic24pt7b.h
        lib/Adafruit-GFX-Library/Fonts/FreeSerifItalic9pt7b.h
        lib/Adafruit-GFX-Library/Fonts/TomThumb.h
        lib/Adafruit-GFX-Library/Adafruit_GFX.cpp
        lib/Adafruit-GFX-Library/Adafruit_GFX.h
        lib/Adafruit-GFX-Library/gfxfont.h
        lib/Adafruit-GFX-Library/glcdfont.c
        lib/Adafruit_GFX_Library/fontconvert/fontconvert.c
        lib/Adafruit_GFX_Library/Fonts/FreeMono12pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeMono18pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeMono24pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeMono9pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeMonoBold12pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeMonoBold18pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeMonoBold24pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeMonoBold9pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeMonoBoldOblique12pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeMonoBoldOblique18pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeMonoBoldOblique24pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeMonoBoldOblique9pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeMonoOblique12pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeMonoOblique18pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeMonoOblique24pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeMonoOblique9pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSans12pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSans18pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSans24pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSans9pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSansBold12pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSansBold18pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSansBold24pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSansBold9pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSansBoldOblique12pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSansBoldOblique18pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSansBoldOblique24pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSansBoldOblique9pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSansOblique12pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSansOblique18pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSansOblique24pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSansOblique9pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSerif12pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSerif18pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSerif24pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSerif9pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSerifBold12pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSerifBold18pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSerifBold24pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSerifBold9pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSerifBoldItalic12pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSerifBoldItalic18pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSerifBoldItalic24pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSerifBoldItalic9pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSerifItalic12pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSerifItalic18pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSerifItalic24pt7b.h
        lib/Adafruit_GFX_Library/Fonts/FreeSerifItalic9pt7b.h
        lib/Adafruit_GFX_Library/Fonts/TomThumb.h
        lib/Adafruit_GFX_Library/Adafruit_GFX.cpp
        lib/Adafruit_GFX_Library/Adafruit_GFX.h
        lib/Adafruit_GFX_Library/gfxfont.h
        lib/Adafruit_GFX_Library/glcdfont.c
        lib/Adafruit_NeoMatrix/extras/gamma.c
        lib/Adafruit_NeoMatrix/Adafruit_NeoMatrix.cpp
        lib/Adafruit_NeoMatrix/Adafruit_NeoMatrix.h
        lib/Adafruit_NeoMatrix/gamma.h
        lib/Adafruit_NeoPixel/Adafruit_NeoPixel.cpp
        lib/Adafruit_NeoPixel/Adafruit_NeoPixel.h
        lib/Adafruit_NeoPixel/esp8266.c
        lib/PID/PID.h
        src/sketch/PID.h
        src/sketch/sketch.ino
        src/sketch/MovementInterface.h
        src/sketch/MotorInterface.h
        src/sketch/MotorInterfaceBase.h
        src/sketch/ClassThatKeepsCoordinatesFromDistances.h
        src/sketch/OurRobotSpecs.h
        src/sketch/MovementInterfaceMasterSlave.h
        src/sketch/MovementInterfaceBase.h
        src/sketch/Calibrator.h
        src/sketch/MiscDefinitions.h
        src/sketch/IRPins.h
        src/sketch/Buttons.h
        src/sketch/ObstacleFinder.h
        src/sketch/WireFinder.h
        src/sketch/Display.h)

add_executable(all_func_wheels ${SOURCE_FILES})