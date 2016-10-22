import threading,os,sys,serial,time

#class used to update 8x8 grid, 7 segment display, 
#and handle GO and STOP buttons


class Interface():

	def __init__(self,display):
		self.display