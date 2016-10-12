import threading,os,sys,serial,time


import Robot

#     directions:
#
#		  1
#		  /\
#  2 <-- ROBOT --> 0
#         \/
#         3
#
class Navigator():
	rows = '1234567'
	cols = 'ABCDEFG'

	#give this control over motors +
	#sensors

	def __init__(self,direction):
		robotMap = RobotMap(direction)
		self.loc = [0,6] #starting block
		self.loc_min = 0
		self.loc_max = 6

	def goToBlock(self,block):
		if block.obstructed:
			return 'OBS'

	def getBestBlock(self,goal_block):
		candidates = []
		#try to get blocks in all directions
		if self.loc[0]+1 <= loc_max:
			candidates.append(robotMap[loc[0]+1,loc[1]])
		if self.loc[1]-1 >= loc_min:
			candidates.append(robotMap[loc[0],loc[1]-1])
		if self.loc[0]-1 <= loc_max:
			candidates.append(robotMap[loc[0]-1,loc[1]])
		if self.loc[1]+1 >= loc_min:
			candidates.append(robotMap[loc[0],loc[1]+1])
		#take out blocks that cannot be chosen
		for block in candidates:
			if block.obstructed:
				candidates.remove(block)
				continue
		#take out blocks that do not decrease distance
		currentDist = robotMap.getDistance(goal_block,robotMap[self.loc])
		distances = []
		for block in candidates:
			distances.append(robotMap.getDistance(goal_block,block))




class RobotMap():
	rows = '1234567'
	cols = 'ABCDEFG'

	#start map with robot facing chosen direction
	def __init__(self,direction):
		self.direction = direction
		self.grid = []
		self.generateBlocks()

	def generateBlocks(self):
		#generate grid
		for col in range(0,7):
			col_list = []
			for row in range(0,7):
				cow_list.append(MapBlock('E',(col,row)))
			self.grid.append(col_list)

		#set perimeter blocks
		# first, the left and right columns
		for block in self.grid[0]:
			block.perimeter = True
		for block in self.grid[6]:
			block.perimeter = True
		# now, the 5 remaining on top and bottom row
		for col in range(1,6):
			self.grid[col][0].perimeter = True
			self.grid[col][6].perimeter = True
		# set bottom left to the starting location
		self.grid[0][6].setStart()
		self.grid[0][6].makeVisited()

	def __getitem__(self,indexes):
		#override [ ] to get index
		return self.grid[indexes[0]][indexes[1]]

	def getDistance(self,block1,block2):
		#total = 0
		#total += abs(block1[0]-block2[0])
		#total += abs(block1[1]-block2[1])

		#distance formula
		total = (((float(block1[0])-block2[0])**2)+((float(block1[1])-block2[1])**2))**(1.0/2)
		return total



class MapBlock():
	#blocks in each grid point

	#types: OT (objective tunnel),
	#       DE (deadend), 
	#       ST (starting position),
	#		E  (empty),
	#		NU (null)

	def __init__(self,type,coords):
		self.type = type
		self.visited = False
		self.obstructed = False
		self.open = False
		self.coords = coords
		self.perimeter = False

	def makeVisited(self):
		self.visited = True

	def removeVisited(self):
		self.visited = False

	def makeObstructed(self):
		self.obstructed = True

	def removeObstructed(self):
		self.obstructed = False

	def setDeadend(self):
		self.type = 'DE'

	def setObjective(self):
		self.type = 'OT'

	def setStart(self):
		self.type = 'ST'

	def setEmpty(self):
		self.type = 'E'

	def getType(self):
		return self.type

	def __getitem__(self,index):
		if index == 0:
			return coords[0]
		elif index == 1:
			return coords[1]
		else:
			pass
