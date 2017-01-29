#get serial library for python-2.7.x
import os,sys,time,threading
import serial

#get robot + robotmap for general movement + navigation
__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__))) #directory from which this script is ran
sys.path.insert(0, os.path.join(__location__,'src/'))
#insert location from Competition Game
#main_dir = os.path.realpath(os.path.join(__location__,'..'))
#sys.path.insert(0, os.path.realpath(os.path.join(main_dir,'Competition Game/src')))


#from SensorArduino import EMF_Sensors
#from Displays import Displays
#from ArduinoFuncs import ArduinoFuncs
from AcousticAnalysis import AcousticAnalysis
#from Robot_Impl import Robot_Impl
#from Robot import RobotMap
#from AI_JED import RobotAlg as AlgJed
#from AI_17 import Robot as Alg17

#ard_serial = serial.Serial('/dev/arduino_allfunc',115200)

#motors = MotorsNXT()
#emf = EMF_Sensors(emf_serial)
#display  = Displays(disp_serial)
#ard_funcs = ArduinoFuncs(ard_serial)
#while not ard_funcs.ard.connected:
#	time.sleep(0.1)

#display = None

globalThreads = []

#initialize Robot_Impl
#direction = 0
#robot_map = RobotMap(None,50,(0,0),direction,shouldPlaySound=False)
#robot_impl = Robot_Impl(ard_funcs,robot_map)
#alg17:
#robotAlgorithm = Alg17(robot_impl,display)
#robotAlgorithm.explore3()

#algJed:
#robotAlgorithm = AlgJed(robot_impl,robot_impl)
#robotAlgorithm.doStuff()

acoustic = AcousticAnalysis(None,'svc_calib','svc_calib_scaler')

while True:
	raw_input("")
	recordReturn = [None,None]
	acousticThread = threading.Thread(target=acoustic.recordDuration,args=(0.300,5,os.path.join(acoustic.calib_location,'wavfiles'),None,recordReturn))
	acousticThread.daemon = True
	acousticThread.start()
	while not acoustic.doneRecording:
		time.sleep(0.2)
	print recordReturn


for thread in globalThreads:
	thread.stopThread()
