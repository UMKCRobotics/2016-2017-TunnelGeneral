# get serial library for python-2.7.x
import os
import serial
import sys
import time

# get robot + robotmap for general movement + navigation
__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__)))
# ^ directory from which this script is run
sys.path.insert(0, os.path.join(__location__, 'src/'))
# insert location from Competition Game
main_dir = os.path.realpath(os.path.join(__location__, '..'))
sys.path.insert(0, os.path.realpath(os.path.join(main_dir, 'Competition Game/src')))

# from SensorArduino import EMF_Sensors
# from Displays import Displays
from ArduinoFuncs import ArduinoFuncs
from Robot_Impl import Robot_Impl
from Robot import RobotMap
from AI_JED import RobotAlg as AlgJed
from AI_17 import Robot as Alg17

ard_serial = serial.Serial('/dev/arduino_allfunc', 115200)
ard_serial.setDTR(level=False)  # http://stackoverflow.com/questions/11385915
time.sleep(2)

# motors = MotorsNXT()
# emf = EMF_Sensors(emf_serial)
# display  = Displays(disp_serial)
ard_funcs = ArduinoFuncs(ard_serial)
while not ard_funcs.ard.connected:
    time.sleep(0.1)

display = None

globalThreads = []

# initialize Robot_Impl
direction = 0
robot_map = RobotMap(None, 50, (0, 0), direction, shouldPlaySound=False)
robot_impl = Robot_Impl(ard_funcs, robot_map)
# alg17:
robotAlgorithm = Alg17(robot_impl, robot_impl)
robotAlgorithm.explore3()

# algJed:
# robotAlgorithm = AlgJed(robot_impl, robot_impl)
# robotAlgorithm.doStuff()

for thread in globalThreads:
    thread.stopThread()
