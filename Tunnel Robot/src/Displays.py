import threading,os,sys,serial,time

from SerialComm import SerialComm
from DeviceComm import CommRequest


class Displays():

	def __init__(self,serial):
		self.ard = SerialComm(serial)
		self.ard.start()

	def set8x8(self,coord,gridType):
		#types: 
		# T = objective tunnel,
		# D = deadend,
		# E = empty
		index = self.convertToIndex(coord)
		print index

		commandObj = CommRequest(gridType + '|' + str(index))
		self.ard.requestCommand(commandObj)
		return commandObj

	def set7segment(self,number):
		if number < 1 or number > 6:
			return 'INVALID'

		commandObj = CommRequest('N' + '|' + str(number))
		self.ard.requestCommand(commandObj)
		return commandObj


	def convertToIndex(self,coord):
		x = coord[0]
		y = coord[1]

		index = y*8 + x + 8

		return index