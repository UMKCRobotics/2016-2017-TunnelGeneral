#get serial library for python-2.7.x
import os,sys,time
import serial

#get robot + robotmap for general movement + navigation
__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__))) #directory from which this script is ran
sys.path.insert(0, os.path.join(__location__,'src/'))

#from SensorArduino import EMF_Sensors
from Displays import Displays

disp_serial = serial.Serial('/dev/ttyUSB0',115200)

#motors = MotorsNXT()
#emf = EMF_Sensors(emf_serial)
display  = Displays(disp_serial)

while not display.ard.connected:
	time.sleep(0.1)

globalThreads = []


def wait_till_done(resp):
	intermediateDelay = 0.01
	while not resp.isDone:
		time.sleep(intermediateDelay)


delayTime = 0.01
intermediateDelay = 0.01

resp = display.set7segment(3);
wait_till_done(resp)

coordinate = (0,5)
resp = display.set8x8(coordinate,'T');
wait_till_done(resp)
time.sleep(delayTime)

resp = display.set8x8(coordinate,'E');
wait_till_done(resp)

coordinate = (0,4)
resp = display.set8x8(coordinate,'T');
wait_till_done(resp)
time.sleep(delayTime)

resp = display.set8x8(coordinate,'E');
wait_till_done(resp)

coordinate = (0,3)
resp = display.set8x8(coordinate,'T');
wait_till_done(resp)
time.sleep(delayTime)

resp = display.set8x8(coordinate,'E');
wait_till_done(resp)

print 'all done!!'
print resp.getResponse()



for thread in globalThreads:
	thread.stopThread()