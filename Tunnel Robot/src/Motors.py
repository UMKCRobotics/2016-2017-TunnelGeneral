import threading,os,sys,serial,time

import nxt
import usb

__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__))) #directory from which this script is ran
sys.path.insert(0, os.path.join(__location__))

from NXTComm import NXTComm
from SerialComm import SerialComm
from DeviceComm import CommRequest


class MotorsNXT():
	def __init__(self):
		try:
			brick = nxt.locator.find_one_brick(debug=True)
		except Exception,e:
			raise Exception("MotorsNXT Error: %s" % str(e))
		if not brick:
			raise Exception("MotorsNXT Error: could not find a brick")

		self.nxtC = NXTComm(brick)
		self.nxtC.valid_commands = ['f','b']
		self.nxtC.start()

	def moveForward(self):
		commandObj = CommRequest('f')
		self.nxtC.requestCommand(commandObj)
		return commandObj

	def moveBackward(self):
		commandObj = CommRequest('b')
		self.nxtC.requestCommand(commandObj)
		return commandObj

	def stopThread(self):
		self.nxtC.stopThread()



class MotorsArduino(SerialComm):
	def __init__(self,serial):
		SerialComm.__init__(self,serial)