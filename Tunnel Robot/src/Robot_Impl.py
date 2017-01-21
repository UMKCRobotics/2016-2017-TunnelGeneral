import threading,os,sys,serial,time

import ArduinoFuncs
from AcousticAnalysis import AcousticAnalysis
from DeviceComm import CommRequest

#class responsible for interfacing with physical robot

class Robot_Impl():

	def __init__(self,arduinofuncs,map_in):
		self.arduinofuncs = arduinofuncs
		self.MAP = map_in
		self.acoustics = AcousticAnalysis(self.arduinofuncs,'svc_calib','svc_calib_scaler')
		self.EMF_thresh = 50
		
	def getGoButton(self):
		return self.arduinofuncs.getGoButton()
	
	def getStopButton(self):
		return self.arduinofuncs.getStopButton()

	def setReadyLight(self):
		return self.arduinofuncs.setReadyLight()

	def set8x8(self,index,gridType):
		# types:
        # T = objective tunnel,
        # D = dead end,
        # E = empty
		return self.arduinofuncs.set8x8(index,gridType)

	def set7segment(self,number):
		return self.arduinofuncs.set7segment(number)

	def goForward(self):
		# forward 1 foot, returns command object
		self.MAP.drive(1)
		return self.arduinofuncs.moveForward()

	def rotateCounterClockwise(self):
		# left 90 degrees, returns command object
		self.MAP.rotateCounterClockwise()
		return self.arduinofuncs.moveLeft()

	def rotateClockwise(self):
		# right 90 degrees, returns command object
		self.MAP.rotateClockwise()
		return self.arduinofuncs.moveRight()

	def goCalibrate(self):
		# use perimeter to fix possible rotation/translation errors
		return self.arduinofuncs.goCalibrate()
	
	def goCalibrateIR(self,side):
		return self.arduinofuncs.goCalibrateIR(side)

	def readSensor(self, value):
		commObj = None
		if value == 1:
			commObj = self.getObstacleReport()
		elif value == 2:
			commObj = self.getEMFreading()
		elif value == 3:
			commObj = self.getIfFoam()
		commObj.markDone()
		return commObj

	def getEMFreading(self):
		# get readings from EMF sensor
		# uncomment next line if has working EMF set up
		# return self.arduinofuncs.getEMFreading()
		commandObj = CommRequest('S' + '|E')
		commandObj.setResponse([0])
		return commandObj


	def getIfFoam(self):
		# get readings from Capacitive sensor(s) via a list
		commandObj = CommRequest('S' + '|F')
		commandObj.setResponse([0])
		return commandObj

	def getIfFoamInProg(self):
		return int(self.acoustics.getIfFoam())

	def getObstacleReport(self):
		# get locations of obstacles in adjacent blocks via a list
		# right, front, left
		# uncomment next line when obstacles CAN be dectected:
		# return self.arduinofuncs.getObstacleReport()
		# leave the next line uncommented if CANNOT detect obstacles:
		commandObj = CommRequest('S' + '|O')
		commandObj.setResponse([0,0,0,0])
		return commandObj
