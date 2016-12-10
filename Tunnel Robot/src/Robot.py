import threading,os,sys,serial,time

import Motors
import SensorArduino

#class responsible for interfacing with physical robot

class Robot():


	def __init__(self,motors,sensors,map_in):
		self.motors = motors
		self.sensors = sensors
		self.MAP = map_in

	def goForward(self):
		#forward 1 foot, returns command object
		return motors.moveForward()

	def rotateCounterClockwise(self):
		#left 90 degrees, returns command object
		return motors.moveLeft()

	def rotateClockwise(self):
		#right 90 degrees, returns command object
		return motors.moveRight()

	def moveCalibrate(self):
		#use perimeter to fix possible rotation/translation errors
		return motors.goCalibrate()

	def getEMF_Reading(self):
		#get readings from EMF sensor(s) via a list
		return sensors.checkEMF()

	def getCapacitive_Reading(self):
		#get readings from Capacitive sensor(s) via a list
		return sensors.checkCapacitive()

	def getObstacle_Reading(self):
		#get locations of obstacles in adjacent blocks via a list
		return sensors.checkObstacles()