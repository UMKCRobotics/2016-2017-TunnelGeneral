import threading,os,sys,serial,time
#specific to NXT comm:
try:
	import nxt
	import usb
except ImportError:
	print 'NXT or USB library not installed; no problemo'

__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__))) #directory from which this script is ran
sys.path.insert(0, os.path.join(__location__))

from DeviceComm import DeviceComm,CommRequest


class NXTComm(DeviceComm):
	# class for communication with NXT brick
	# probably only used as demo
	valid_commands = []
	def __init__(self,comm):
		DeviceComm.__init__(self,comm)
		self.brick = self.comm

	def performCommand(self,commReq):
		if commReq.request[0] in self.valid_commands:
			self.brick.message_write(0,commReq.request)
		errorEncountered = True
		while errorEncountered:
			try:
				local_box, message = self.brick.message_read(1,1,True)
				errorEncountered = False
				print message
			except nxt.error.DirProtError:
				pass
		print "command performed"
		commReq.markDone()
		