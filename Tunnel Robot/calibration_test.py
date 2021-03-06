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
from ArduinoFuncs import ArduinoFuncs
from Robot_Impl import Robot_Impl
from Robot import RobotMap
from AI_JED import RobotAlg as AlgJed
from AI_17 import Robot as Alg17

ard_serial = serial.Serial('/dev/arduino_allfunc',115200)

#motors = MotorsNXT()
#emf = EMF_Sensors(emf_serial)
#display  = Displays(disp_serial)
ard_funcs = ArduinoFuncs(ard_serial)
while not ard_funcs.ard.connected:
	time.sleep(0.1)


def wait_till_done(resp):
        intermediateDelay = 0.01
        while not resp.isDone:
            time.sleep(intermediateDelay)
        return resp.getResponse()

display = None

globalThreads = []

#initialize Robot_Impl
direction = 0
robot_map = RobotMap(None,50,(0,0),direction,shouldPlaySound=False)
robot_impl = Robot_Impl(ard_funcs,robot_map)
#alg17:
#robotAlgorithm = Alg17(robot_impl,display)
#robotAlgorithm.explore3()

#algJed:
#robotAlgorithm = AlgJed(robot_impl,robot_impl)
#robotAlgorithm.doStuff()
print 'Ready for commands'
while True:
	user_inp = raw_input('> ')
	if user_inp == 'cl':
		print wait_till_done(robot_impl.goCalibrateIR('L'))
	elif user_inp == 'cr':
		print wait_till_done(robot_impl.goCalibrateIR('R'))
	elif user_inp == 'cb':
		print wait_till_done(robot_impl.goCalibrateIR('B'))
	elif user_inp == 'cs':
		print wait_till_done(robot_impl.goCalibrate())
	elif user_inp == 'f':
		print wait_till_done(robot_impl.goForward())
	elif user_inp == 'r':
		print wait_till_done(robot_impl.rotateClockwise())
	elif user_inp == 'l':
		print wait_till_done(robot_impl.rotateCounterClockwise())
	elif user_inp == 't':
		print wait_till_done(robot_impl.arduinofuncs.performTap())
	elif user_inp == 'o':
		print wait_till_done(robot_impl.getObstacleReport())
	elif user_inp == 'exit':
		break
	

for thread in globalThreads:
	thread.stopThread()
