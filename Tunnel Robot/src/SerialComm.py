import threading,os,sys,serial,time

__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__))) #directory from which this script is ran
sys.path.insert(0, os.path.join(__location__))

from DeviceComm import DeviceComm,CommRequest



class SerialComm(DeviceComm):
	# class for communication with devices
	# via serial communication
	# such as: arduinos
	def __init__(self,comm):
		DeviceComm.__init__(self,comm)
		#alias for comm
		self.serial = self.comm

	def performCommand(self,commReq):
		pass
		#insert stuff here