#get serial library for python-2.7.x
import os,sys,time
import serial

#get robot + robotmap for general movement + navigation
__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__))) #directory from which this script is ran
sys.path.insert(0, os.path.join(__location__,'src/'))
#insert location from Competition Game
main_dir = os.path.realpath(os.path.join(__location__,'..'))
sys.path.insert(0, os.path.realpath(os.path.join(main_dir,'Competition Game/src')))


#from SensorArduino import EMF_Sensors
#from Displays import Displays
from Motors import MotorsArduino
from Displays import Displays
from Robot_Impl import Robot_Impl
from Robot import RobotMap
from AI_JED import RobotAlg as AlgJed
from AI_17 import Robot as Alg17

motor_serial = serial.Serial('/dev/ttyUSB0',115200)

#motors = MotorsNXT()
#emf = EMF_Sensors(emf_serial)
#display  = Displays(disp_serial)
motors = MotorsArduino(motor_serial)
while not motors.ard.connected:
	time.sleep(0.1)

display = None

globalThreads = []

#initialize Robot_Impl
direction = 0
robot_map = RobotMap(None,0,None,None,direction,shouldPlaySound=False)
robot_impl = Robot_Impl(motors,display,robot_map)
#alg17:
robotAlgorithm = Alg17(robot_impl,display)
#robotAlgorithm.explore3()

#algJed:
#robotAlgorithm = AlgJed(robot_impl,display)
#robotAlgorithm.doStuff()


for thread in globalThreads:
	thread.stopThread()