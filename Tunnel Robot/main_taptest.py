#get serial library for python-2.7.x
import os,sys,time
import serial

#get robot + robotmap for general movement + navigation
__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__))) #directory from which this script is ran
sys.path.insert(0, os.path.join(__location__,'src/'))

#from SensorArduino import EMF_Sensors
from Displays import Displays

disp_serial = serial.Serial('/dev/ttyACM0',115200)

#motors = MotorsNXT()
#emf = EMF_Sensors(emf_serial)
display  = Displays(disp_serial)

while not display.isConnected():
	time.sleep(0.1)

print 'done connecting'

globalThreads = []

def translate_coordinate_to_index(coord):
    """
    change a coordinate to the index on the 8x8 display
    index is 0 in top left, counting up to the right

    :param coord: a coordinate using this robot brain's system of coordinates
    :return: int - the index to be shown on the display
    """
    x = coord[0]
    y = coord[1]

    index = y * 8 + x + 8

    return index


def wait_till_done(resp):
	intermediateDelay = 0.01
	while not resp.isDone:
		time.sleep(intermediateDelay)


delayTime = 0.01
intermediateDelay = 0.01

for i in range(0,30):
    resp = display.performTap()
    wait_till_done(resp)



print 'done now!'




for thread in globalThreads:
	thread.stopThread()