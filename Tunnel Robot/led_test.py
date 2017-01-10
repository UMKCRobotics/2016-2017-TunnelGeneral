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
from DeviceComm import CommRequest
from Robot import RobotMap
from AI_JED import RobotAlg as AlgJed
from AI_17 import Robot as Alg17

ard_serial = serial.Serial('/dev/ttyACM0',115200)

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
resp = "1"
automatic = True
index = 0
possibilities = ['y','b','g']
while True:
        if automatic:
                if resp.startswith("BAD") or resp.startswith("BAd"):
                        automatic = False
                        print "was on: %s" % possibilities[index]
                        print 'attempting to flush...'
                        time.sleep(0.5)
                        ard_funcs.ard.serial.flushInput()
                        ard_funcs.ard.serial.flushOutput()
                        continue
                index = (index+1)%3
                user_inp = possibilities[index]
                #time.sleep(0.001)
	else:
                user_inp = raw_input('> ')
	if user_inp == 'y':
                commandObj = CommRequest("L|Y")
                ard_funcs.ard.requestCommand(commandObj)
		resp = wait_till_done(commandObj)
		print "%s: %s" % (user_inp,resp) 
	elif user_inp == 'b':
                commandObj = CommRequest("L|B")
                ard_funcs.ard.requestCommand(commandObj)
		resp = wait_till_done(commandObj)
		print "%s: %s" % (user_inp,resp) 
	elif user_inp == 'g':
                commandObj = CommRequest("L|G")
                ard_funcs.ard.requestCommand(commandObj)
		resp = wait_till_done(commandObj)
		print "%s: %s" % (user_inp,resp) 
	elif user_inp == 'n':
                commandObj = CommRequest("L|N")
                ard_funcs.ard.requestCommand(commandObj)
		resp = wait_till_done(commandObj)
		print "%s: %s" % (user_inp,resp) 
	elif user_inp == 'a':
                automatic = True
        elif user_inp == 'flush':
                ard_funcs.ard.serial.flushInput()
                ard_funcs.ard.serial.flushOutput()
	elif user_inp == 'exit':
		break
        else:
                commandObj = CommRequest("mmm")
                ard_funcs.ard.requestCommand(commandObj)
		resp = wait_till_done(commandObj)
		print resp 
	

for thread in globalThreads:
	thread.stopThread()
