import threading,os,sys,serial,time

__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__))) #directory from which this script is ran
sys.path.insert(0, os.path.join(__location__))

from DeviceComm import DeviceComm,CommRequest

# formatting:
# request = "<command char>\n"
# response = "d<response stuff...>\n"


class SerialComm(DeviceComm):
	# class for communication with devices
	# via serial communication
	# such as: arduinos
	termChar = '\n'

	def __init__(self,comm):
		DeviceComm.__init__(self,comm)
		#alias for comm
		self.serial = self.comm
		self.connected = False
		self.initializeConnection()

	def initializeConnection(self):
		#wait until serial is connected
		while not self.connected:
			time.sleep(0.1)
			self.serial.timeout = 1 #make serial non blocking
			serin = self.serial.read()
			if len(serin) > 0:
				self.connected = True
		self.serial.timeout = None #make serial block
		print 'connection initialized'

	def performCommand(self,commReq):
		#print 'perfoming command --> <<<%s>>>' % (commReq.request)
		self.serial.write(commReq.request+'\n')
		#print 'waiting for response...'
		#print self.serial
		#wait for a response to change state of command Request
		serin = ""
		while serin.endswith('\n') != True:
			serin += self.serial.read()
			#time.sleep(0.1)
		#print serin
		if serin[0] == 'n':
			response = 'BAD'
		else:
			response = serin.strip()[1:] #remove new line char, take out first char
			#print response
		#print 'command %s performed with response %s' % (commReq.request,response)
		commReq.response = response #set response
		commReq.markDone()
		#insert stuff here