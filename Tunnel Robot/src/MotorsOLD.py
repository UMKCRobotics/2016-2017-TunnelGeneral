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
		self.nxtC.valid_commands = ['f','b','l','r']
		self.nxtC.start()

	def moveForward(self):
		commandObj = CommRequest('f')
		self.nxtC.requestCommand(commandObj)
		return commandObj

	def moveBackward(self):
		commandObj = CommRequest('b')
		self.nxtC.requestCommand(commandObj)
		return commandObj

	def moveLeft(self):
		commandObj = CommRequest('l')
		self.nxtC.requestCommand(commandObj)
		return commandObj

	def moveRight(self):
		commandObj = CommRequest('r')
		self.nxtC.requestCommand(commandObj)
		return commandObj

	def stopThread(self):
		self.nxtC.stopThread()



class MotorsArduino():
	def __init__(self,serial):
		self.ard = SerialComm(self,serial)

	def moveForward(self):
		commandObj = CommRequest('f')
		self.ard.requestCommand(commandObj)
		return commandObj

	def moveBackward(self):
		commandObj = CommRequest('b')
		self.ard.requestCommand(commandObj)
		return commandObj

	def moveLeft(self):
		commandObj = CommRequest('l')
		self.ard.requestCommand(commandObj)
		return commandObj

	def moveRight(self):
		commandObj = CommRequest('r')
		self.ard.requestCommand(commandObj)
		return commandObj

	def stopThread(self):
		self.ard.stopThread()