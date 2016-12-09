#get serial library for python-2.7.x
import os,sys,time
import serial
#stuff
#get robot + robotmap for general movement + navigation
__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__))) #directory from which this script is ran
sys.path.insert(0, os.path.join(__location__,'src/'))

#from SensorArduino import EMF_Sensors
from Displays import Displays

disp_serial = serial.Serial('/dev/ttyUSB0',115200)

#motors = MotorsNXT()
#emf = EMF_Sensors(emf_serial)
display  = Displays(disp_serial)

while not display.isConnected():
	time.sleep(0.1)

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

for i in range(1,7):
	resp = display.set7segment(i);
	wait_till_done(resp)
	print resp.getResponse()
	time.sleep(0.25)

coordinate = translate_coordinate_to_index((0,5))
resp = display.set8x8(coordinate,'T');
wait_till_done(resp)
time.sleep(delayTime)

resp = display.set8x8(coordinate,'E');
wait_till_done(resp)

coordinate = translate_coordinate_to_index((0,4))
resp = display.set8x8(coordinate,'T');
wait_till_done(resp)
time.sleep(delayTime)

resp = display.set8x8(coordinate,'E');
wait_till_done(resp)

coordinate = translate_coordinate_to_index((0,3))
resp = display.set8x8(coordinate,'T');
wait_till_done(resp)
time.sleep(delayTime)

#resp = display.set8x8(coordinate,'E');
#wait_till_done(resp)

print 'all done!!'
print resp.getResponse()

time.sleep(1);

resp = display.getGoButton();
wait_till_done(resp)
time.sleep(delayTime)
print "GO STATE: " + resp.getResponse()
resp = display.getStopButton();
wait_till_done(resp)
time.sleep(delayTime)
print "STOP STATE: " + resp.getResponse()




for thread in globalThreads:
	thread.stopThread()