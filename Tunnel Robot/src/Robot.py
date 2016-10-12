import threading,os,sys,serial,time

import Motors
import SensorArduino

#class responsible for interfacing with physical robot

class Robot():


	def __init__(self):
		self.motors = None
		self.sensors = None

	def moveForward(self):
		#forward 1 foot, returns command object
		pass

	def moveLeft(self):
		#left 90 degrees, returns command object
		pass

	def moveRight(self):
		#right 90 degrees, returns command object
		pass

	def moveCalibrate(self):
		#use perimeter to fix possible rotation/translation errors
		pass

	def getEMF_Reading(self):
		#get readings from EMF sensor(s) via a list
		pass

	def getCapacitive_Reading(self):
		#get readings from Capacitive sensor(s) via a list
		pass

	def getObstacle_Reading(self):
		#get locations of obstacles in adjacent blocks via a list
		pass