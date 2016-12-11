import threading,os,sys,serial,time

import Motors
import SensorArduino
from DeviceComm import CommRequest

#class responsible for interfacing with physical robot

class Robot_Impl():


	def __init__(self,arduinofuncs,map_in):
		self.arduinofuncs = arduinofuncs
		self.MAP = map_in

	def goForward(self):
		#forward 1 foot, returns command object
		self.MAP.drive(1)
		return arduinofuncs.moveForward()

	def rotateCounterClockwise(self):
		#left 90 degrees, returns command object
		self.MAP.rotateCounterClockwise()
		return arduinofuncs.moveLeft()

	def rotateClockwise(self):
		#right 90 degrees, returns command object
		self.MAP.rotateClockwise()
		return arduinofuncs.moveRight()

	def goCalibrate(self):
		#use perimeter to fix possible rotation/translation errors
		return arduinofuncs.goCalibrate()

	def readSensor(self, value):
		commObj = None
		if value == 1:
			commObj = getObstacle_Reading()
		elif value == 2:
			commObj = getIfFoam()
		elif value == 3:
			commObj = getObstacle_Reading()
		commObj.markDone()
		return commObj

	def getEMF_Reading(self):
		#get readings from EMF sensor(s) via a list
		commandObj = CommRequest('S' + '|E')
		commandObj.setResponse('0')
		return commandObj

	def getIfFoam(self):
		#get readings from Capacitive sensor(s) via a list
		commandObj = CommRequest('S' + '|F')
		commandObj.setResponse('0')
		return commandObj

	def getObstacle_Reading(self):
		#get locations of obstacles in adjacent blocks via a list
		#right, front, left
		commandObj = CommRequest('S' + '|O')
		commandObj.setResponse('000')
		return commandObj