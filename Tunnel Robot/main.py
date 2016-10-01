#get serial library for python-2.7.x
import os,sys,time
import serial

#get robot + robotmap for general movement + navigation
__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__))) #directory from which this script is ran
sys.path.insert(0, os.path.join(__location__,'src/'))

from Motors import MotorsNXT
from SensorArduino import EMF_Sensors

emf_serial = serial.Serial('/dev/ttyACM0',115200)

motors = MotorsNXT()
emf = EMF_Sensors(emf_serial)

while not emf.ard.connected:
	time.sleep(0.1)

globalThreads = []
globalThreads.append(motors)

#resp = motors.moveForward()
#resp2 = motors.moveBackward()
#resp3 = motors.moveLeft()
#resp4 = motorsself.moveRight()
for i in range(0,3):
	resp = motors.moveForward()

print 'EMF commands: %s' % str(emf.ard.commandList)



print 'we still doin stuff'

#while True:
#	if resp.checkDone():
#		print 'done!'
#		break
#	time.sleep(0.1)

#while True:
#	if resp2.checkDone():
#		print 'done2!'
#		break
#	time.sleep(0.1)

#while True:
#	if resp3.checkDone():
#		print 'done3!'
#		break
#	time.sleep(0.1)

#while True:
#	if resp4.checkDone():
#		print 'done4!'
#		break
#	time.sleep(0.1)

emf_data = []
data = None


while True:
	if resp.checkDone():
		print 'done!'
		if data:
			print 'waiting for data to be saved...'
			while not data.checkDone():
				time.sleep(0.01)
		break
	else:
		if data == None:
			data = emf.checkEMF1()
		else:
			if data.checkDone():
				emf_data.append(data.response)
				data = None
	time.sleep(0.01)
data = emf.checkEMF1()
while not data.checkDone():
#	print 'not done yet...'
	time.sleep(0.01)
emf_data.append(data.response)
#data = emf.checkEMF1()


print emf_data

for thread in globalThreads:
	thread.stopThread()