#get serial library for python-2.7.x
import os,sys,time
import serial

#get robot + robotmap for general movement + navigation
__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__))) #directory from which this script is ran
sys.path.insert(0, os.path.join(__location__,'src/'))

from Motors import MotorsNXT

motors = MotorsNXT()

globalThreads = []
globalThreads.append(motors)

resp = motors.moveForward()
resp2 = motors.moveBackward()
print 'we still doin stuff'

while True:
	if resp.checkDone():
		print 'done!'
		break
	time.sleep(0.1)

while True:
	if resp2.checkDone():
		print 'done2!'
		break
	time.sleep(0.1)

for thread in globalThreads:
	thread.stopThread()