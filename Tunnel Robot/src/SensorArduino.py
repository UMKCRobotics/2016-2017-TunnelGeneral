import threading,os,sys,serial,time


#__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__))) #directory from which this script is ran
#sys.path.insert(0, os.path.join(__location__))

from SerialComm import SerialComm
from DeviceComm import CommRequest


class EMF_Sensors():


	def __init__(self,serial):
		self.ard = SerialComm(serial)
		self.ard.start()
		#remove bad data
		self.checkEMF1()

	def checkEMF1(self):
		commandObj = CommRequest('1')
		self.ard.requestCommand(commandObj)
		return commandObj

	def checkEMF2(self):
		commandObj = CommRequest('2')
		self.ard.requestCommand(commandObj)
		return commandObj