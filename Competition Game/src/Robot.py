import pygame, sys, os, random, math, time, threading

__location__ = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))  # directory from which this script is ran
print "location:: %s" % __location__

main_dir = os.path.realpath(os.path.join(__location__,'../..'))
sys.path.insert(0, os.path.realpath(os.path.join(main_dir,'Tunnel Robot/src')))

from DeviceComm import CommRequest

# sys.path.insert(0, os.path.join(__location__))
# SOUND_FOLDER = os.path.join(__location__,'soundfx')
# print "sound_folder:: %s" % SOUND_FOLDER


class Robot():
    coords = None
    width = None

    def __init__(self, screen, gameboard, coords=None, offsets=None):
        global __location__
        self.screen = screen
        self.coords = coords
        self.GAMEBOARD = gameboard
        self.GRID_WIDTH = gameboard.GRID_WIDTH
        gameboard.ROBOT = self  # add self to gameboard
        if coords is None:
            A7_coords = gameboard.get_block('A7').coords
            self.coords = (A7_coords[0] + self.GRID_WIDTH / 8, A7_coords[1] + self.GRID_WIDTH / 8)
        self.width = self.GRID_WIDTH * 3 / 4
        self.color = (26, 148, 49)
        self.object = pygame.Rect(self.coords, (self.width, self.width))
        self.movable_rect = pygame.Rect((gameboard.PLEXI.x + self.GRID_WIDTH / 8,
                                         gameboard.PLEXI.y + self.GRID_WIDTH / 8),
                                        (gameboard.PLEXI.width - self.GRID_WIDTH / 4,
                                         gameboard.PLEXI.height - self.GRID_WIDTH / 4))
        self.direction = 0
        self.dirIndicator = pygame.Rect((self.coords), (10, 10))
        self.dirIndicatorColor = (100, 100, 100)
        # self.errorMax = self.GRID_WIDTH/12
        # self.errorMax = self.GRID_WIDTH/3
        self.errorMax = 0

        #command list to simulate actual Tunnel Robot code
        self.commandList = []

        # move counters
        self.turnCounter = 0
        self.forwardCounter = 0

        self.MAP = None
        if offsets is None:
            self.MAP = RobotMap(self.screen, self.GRID_WIDTH / 2, (gameboard.TOTAL_WIDTH + 20, 20), self.direction)
        else:
            self.MAP = RobotMap(self.screen, self.GRID_WIDTH / 2, offsets, self.direction)
        self.rel_coords = (self.coords[0] - self.GAMEBOARD.OFFSETS[0], self.coords[1] - self.GAMEBOARD.OFFSETS[1])
        self.last_reference = [0, 0]
        self.sensors = []
        self.leftA7 = False
        # used for timing robot movement
        self.counter = 1
        self.counter_max = 144
        # load sounds
        self.SOUND_FOLDER = os.path.join(__location__, 'soundfx')
        self.sound1 = pygame.mixer.Sound(os.path.join(self.SOUND_FOLDER, 'smb_jump-small.wav'))
        self.sound2 = pygame.mixer.Sound(os.path.join(self.SOUND_FOLDER, 'smb_jump-super.wav'))
        self.sound3 = pygame.mixer.Sound(os.path.join(self.SOUND_FOLDER, 'smb_coin.wav'))
        self.sound4 = pygame.mixer.Sound(os.path.join(self.SOUND_FOLDER, 'smb_fireball.wav'))
        self.sound5 = pygame.mixer.Sound(os.path.join(self.SOUND_FOLDER, 'smb_stomp.wav'))
        # self.song = pygame.mixer.Sound(os.path.join(self.SOUND_FOLDER,'01-super-mario-bros.wav'))
        pygame.mixer.music.load(os.path.join(self.SOUND_FOLDER, '01-super-mario-bros.wav'))
        pygame.mixer.music.play(-1)

    # self.play_sound(self.song)

    def play_sound(self, sound):
        soundT = threading.Thread(target=self.__play_sound__, args=(sound,))
        soundT.daemon = True
        soundT.start()

    def play_song(self, song):
        soundT = threading.Thread(target=self.__play_song__, args=(song,))
        soundT.daemon = True
        soundT.start()

    @staticmethod
    def __play_sound__(sound):
        sound.play()
        time.sleep(sound.get_length())

    @staticmethod
    def __play_song__(sound):
        sound.play()
        time.sleep(sound.get_length())

    def draw(self):
        self.object.topleft = (self.GAMEBOARD.OFFSETS[0] + self.rel_coords[0],
                               self.GAMEBOARD.OFFSETS[1] + self.rel_coords[1])
        self.movable_rect.topleft = (self.GAMEBOARD.PLEXI.x + self.GRID_WIDTH / 8,
                                     self.GAMEBOARD.PLEXI.y + self.GRID_WIDTH / 8)
        self.screen.fill(self.color, self.object)
        self.dirIndicator.center = self.object.center
        if self.direction == 0:
            self.dirIndicator.centerx += self.object.width / 3
        elif self.direction == 1:
            self.dirIndicator.centery -= self.object.height / 3
        elif self.direction == 2:
            self.dirIndicator.centerx -= self.object.width / 3
        elif self.direction == 3:
            self.dirIndicator.centery += self.object.height / 3
        self.screen.fill(self.dirIndicatorColor, self.dirIndicator)
        for sensor in self.sensors:
            sensor.draw()
        # draw map
        # self.MAP.draw()

    def handleKeyEvent(self, event):
        if event.key in [pygame.K_w, pygame.K_UP]:
            self.goForward()
        elif event.key in [pygame.K_a, pygame.K_LEFT]:
            self.rotateCounterClockwise()
        elif event.key in [pygame.K_s, pygame.K_DOWN]:
            self.goBackward()
        elif event.key in [pygame.K_d, pygame.K_RIGHT]:
            self.rotateClockwise()
        elif event.key in [pygame.K_1]:
            self.readSensor(1)
        elif event.key in [pygame.K_2]:
            self.readSensor(2)
        elif event.key in [pygame.K_3]:
            self.readSensor(3)
        elif event.key in [pygame.K_u]:
            if pygame.key.get_mods() & pygame.KMOD_SHIFT:
                self.MAP.markOT_Front()
            else:
                self.MAP.markOT()
        elif event.key in [pygame.K_i]:
            if pygame.key.get_mods() & pygame.KMOD_SHIFT:
                self.MAP.markDeadend_Front()
            else:
                self.MAP.markDeadend()
        elif event.key in [pygame.K_o]:
            if pygame.key.get_mods() & pygame.KMOD_SHIFT:
                self.MAP.markEmpty_Front()
            else:
                self.MAP.markEmpty()
        elif event.key == pygame.K_r:
            self.attemptReference()

    def readSensor(self, value):
        commObject = CommRequest('S%s' % value)
        returnvalue = None
        if value == 1:
            returnvalue = self.read_distance()
        elif value == 2:
            returnvalue = self.read_electromagnetic()
        elif value == 3:
            returnvalue = self.read_capacitive()
        commObject.response = returnvalue
        commObject.markDone()
        return commObject


    def read_distance(self):
        readings = []
        for sensor in self.sensors:  # checks all sensors
            if isinstance(sensor, Distance_Sensor):
                collision_list = []
                for item in self.GAMEBOARD.obstructions:
                    collision_list.append(item.object)
                readings.append(sensor.read_sensor(collision_list))
        print str(readings)
        return readings

    def read_electromagnetic(self):
        readings = []
        for sensor in self.sensors:
            if isinstance(sensor, Electromagnetic_Sensor):
                collision_list = []
                for item in self.GAMEBOARD.powerlines:
                    collision_list.append(item.object)
                readings.append(sensor.read_sensor(collision_list))
        print str(readings)
        return readings

    def read_capacitive(self):
        readings = []
        for sensor in self.sensors:
            if isinstance(sensor, Capacitive_Sensor):
                collision_list = []
                for item in self.GAMEBOARD.empty:
                    collision_list.append(item.object)
                readings.append(sensor.read_sensor(collision_list))
        print str(readings)
        return readings

    def performMove(self):
        # insert code here
        pass

    def goForward(self):
        commObject = CommRequest('f')
        self.drive(1)
        self.forwardCounter += 1
        self.play_sound(self.sound1)
        print "FORWARDS: %s \nTURNS: %s" % (self.forwardCounter,self.turnCounter)
        commObject.markDone()
        return commObject

    def goBackward(self):
        commObject = CommRequest('b')
        self.drive(-1)
        self.forwardCounter += 1
        commObject.markDone()
        return commObject

    def rotateCounterClockwise(self):
        commObject = CommRequest('l')
        self.changeDirection(1)
        self.MAP.rotateCounterClockwise()
        self.turnCounter += 1
        self.play_sound(self.sound5)
        print "FORWARDS: %s \nTURNS: %s" % (self.forwardCounter,self.turnCounter)
        commObject.markDone()
        return commObject

    def rotateClockwise(self):
        commObject = CommRequest('r')
        self.changeDirection(-1)
        self.MAP.rotateClockwise()
        self.turnCounter += 1
        self.play_sound(self.sound5)
        print "FORWARDS: %s \nTURNS: %s" % (self.forwardCounter,self.turnCounter)
        commObject.markDone()
        return commObject

    def changeDirection(self, val):
        self.direction = (self.direction + val) % 4
        #self.MAP.direction = self.direction

    def drive(self, val):
        self.MAP.drive(val)
        # error = 0
        error = round(self.errorMax * (random.random() - 0.5))
        newval = ((self.GRID_WIDTH + error) * val)
        print newval
        if self.direction == 0:
            self.object.x += newval
        elif self.direction == 2:
            self.object.x -= newval
        elif self.direction == 1:
            self.object.y -= newval
        elif self.direction == 3:
            self.object.y += newval

        testObj = pygame.Rect(self.object.topleft, self.object.size)
        self.object.clamp_ip(self.movable_rect)
        # check if actually moved
        if testObj.topleft == self.object.topleft:
            self.last_reference[0] += 1
            self.last_reference[1] += 1

        self.rel_coords = (self.object.x - self.GAMEBOARD.OFFSETS[0], self.object.y - self.GAMEBOARD.OFFSETS[1])
        # check if robot is reference, and on what sides
        if self.object.topleft[0] == self.movable_rect.topleft[0] or self.object.topright[0] == \
                self.movable_rect.topright[0]:
            self.last_reference[0] = 0
        if self.object.bottomleft[1] == self.movable_rect.bottomleft[1] or self.object.topleft[1] == \
                self.movable_rect.topleft[1]:
            self.last_reference[1] = 0

        # check if any obstructions have been moved
        collision_list = []
        for item in self.GAMEBOARD.obstructions:
            collision_list.append(item.object)
        actual_collisions = self.object.collidelistall(collision_list)
        for col_obj in actual_collisions:
            self.GAMEBOARD.obstructions[col_obj].touched = True
            print 'OBSTRUCTION TOUCHED'

        print self.last_reference

        #check if robot left A7 if has not done so yet
        if not self.leftA7 and not self.GAMEBOARD.get_block('A7').object.contains(self.object):
            self.leftA7 = True




# OT Map: Robot's internal map of the world
class RobotMap():
    rows = '1234567'
    cols = 'ABCDEFG'

    def __init__(self, screen, grid_width, offsets, direction,shouldPlaySound=True):
        self.screen = screen
        self.GRID_WIDTH = grid_width
        self.TOTAL_WIDTH = grid_width * 7
        self.TOTAL_HEIGHT = self.TOTAL_WIDTH
        self.grid = []
        self.OFFSETS = offsets
        self.generateBlocks()
        # robot representation stuff
        A7 = self.get_block('A7')
        A7.setStart()
        self.color_ROBOT = (26, 148, 49)
        self.robotMini = pygame.Rect((0, 0), (0, 0))
        self.putRobotInBlock('A7')
        self.robotLoc = [0, 6]
        self.direction = direction
        self.dirIndicator = pygame.Rect((self.robotMini.topleft), (5, 5))
        self.dirIndicatorColor = (100, 100, 100)
        # load sounds
        self.shouldPlaySound = shouldPlaySound
        if self.shouldPlaySound:
            self.SOUND_FOLDER = os.path.join(__location__, 'soundfx')
            self.soundOT = pygame.mixer.Sound(os.path.join(self.SOUND_FOLDER, 'smb_coin.wav'))
            self.soundDeadend = pygame.mixer.Sound(os.path.join(self.SOUND_FOLDER, 'smb_fireball.wav'))

    def play_sound(self, sound):
        if self.shouldPlaySound:
            soundT = threading.Thread(target=self.__play_sound__, args=(sound,))
            soundT.daemon = True
            soundT.start()

    @staticmethod
    def __play_sound__(sound):
        sound.play()
        time.sleep(sound.get_length())

    def generateBlocks(self):
        for col in range(0, 7):
            col_list = []
            for row in range(0, 7):
                col_list.append(MapBlock(self.screen, self, (col * self.GRID_WIDTH,
                                                             row * self.GRID_WIDTH), self.GRID_WIDTH,
                                         self.cols[col] + self.rows[row]))
            self.grid.append(col_list)

    def get_location(self, blockName):
        col = self.cols.find(blockName[0])
        row = self.rows.find(blockName[1])
        return [col, row]

    def make_location(self, coordList):
        col = self.cols[coordList[0]]
        row = self.rows[coordList[1]]
        return col + row

    def get_block(self, blockName):
        col = self.cols.find(blockName[0])
        row = self.rows.find(blockName[1])
        return self.grid[col][row]

    def putRobotInBlock(self, location):
        block = self.get_block(location)
        self.robotMini.x = block.object.x + self.GRID_WIDTH / 8
        self.robotMini.y = block.object.y + self.GRID_WIDTH / 8
        self.robotMini.width = self.GRID_WIDTH * 3 / 4
        self.robotMini.height = self.GRID_WIDTH * 3 / 4

    def markOT(self):
        self.markCurrent('OT')
        self.play_sound(self.soundOT)

    def markEmpty(self):
        self.markCurrent('E')

    def markDeadend(self):
        self.markCurrent('D')
        self.play_sound(self.soundDeadend)

    def markOT_Front(self):
        self.markInFront('OT')

    def markEmpty_Front(self):
        self.markInFront('E')

    def markDeadend_Front(self):
        self.markInFront('D')

    def getBlockInFront(self):
        if self.direction == 0:
            if self.robotLoc[0] < 6:
                reqLoc = (self.robotLoc[0] + 1, self.robotLoc[1])
                return self.get_block(self.make_location(reqLoc))
            else:
                return None
        elif self.direction == 1:
            if self.robotLoc[1] > 0:
                reqLoc = (self.robotLoc[0], self.robotLoc[1] - 1)
                return self.get_block(self.make_location(reqLoc))
            else:
                return None
        elif self.direction == 2:
            if self.robotLoc[0] > 0:
                reqLoc = (self.robotLoc[0] - 1, self.robotLoc[1])
                return self.get_block(self.make_location(reqLoc))
            else:
                return None
        elif self.direction == 3:
            if self.robotLoc[1] < 6:
                reqLoc = (self.robotLoc[0], self.robotLoc[1] + 1)
                return self.get_block(self.make_location(reqLoc))
            else:
                return None

    def markCurrent(self, type):
        if type == 'OT':
            self.get_block(self.make_location(self.robotLoc)).setOT()
        elif type == 'E':
            self.get_block(self.make_location(self.robotLoc)).setEmpty()
        elif type == 'D':
            self.get_block(self.make_location(self.robotLoc)).setDeadend()

    def markInFront(self, type):
        reqBlock = self.getBlockInFront()
        if reqBlock != None:
            if type == 'OT':
                reqBlock.setOT()
            elif type == 'E':
                reqBlock.setEmpty()
            elif type == 'D':
                reqBlock.setDeadend()

    def get_adjacent_blocks(self, block):
        blockLoc = self.get_location(block.loc)
        block_list = []
        if blockLoc[0] >= 0 and blockLoc[0] < 6:
            block_list.append(self.grid[blockLoc[0] + 1][blockLoc[1]])
        if blockLoc[0] > 0 and blockLoc[0] <= 6:
            block_list.append(self.grid[blockLoc[0] - 1][blockLoc[1]])
        if blockLoc[1] >= 0 and blockLoc[1] < 6:
            block_list.append(self.grid[blockLoc[0]][blockLoc[1] + 1])
        if blockLoc[1] > 0 and blockLoc[1] <= 6:
            block_list.append(self.grid[blockLoc[0]][blockLoc[1] - 1])
        return block_list

    def draw(self):
        # draw base board
        self.draw_blocks()
        self.draw_grid()
        self.draw_robot()

    def draw_robot(self):
        self.putRobotInBlock(self.make_location(self.robotLoc))
        self.screen.fill(self.color_ROBOT, self.robotMini)
        self.dirIndicator.center = self.robotMini.center
        if self.direction == 0:
            self.dirIndicator.centerx += self.robotMini.width / 3
        elif self.direction == 1:
            self.dirIndicator.centery -= self.robotMini.height / 3
        elif self.direction == 2:
            self.dirIndicator.centerx -= self.robotMini.width / 3
        elif self.direction == 3:
            self.dirIndicator.centery += self.robotMini.height / 3
        self.screen.fill(self.dirIndicatorColor, self.dirIndicator)

    def draw_blocks(self):
        for col in self.grid:
            for block in col:
                block.draw()

    def draw_grid(self):
        linecolor = (255, 255, 255)
        for n in range(0, 8):
            pygame.draw.lines(self.screen, linecolor, False, [(self.OFFSETS[0] + self.GRID_WIDTH * n, self.OFFSETS[1]),
                                                              (self.OFFSETS[0] + self.GRID_WIDTH * n,
                                                               self.TOTAL_WIDTH + self.OFFSETS[1])], 1)
        for n in range(0, 8):
            pygame.draw.lines(self.screen, linecolor, False, [(self.OFFSETS[0], self.GRID_WIDTH * n + self.OFFSETS[1]),
                                                              (self.OFFSETS[0] + self.TOTAL_WIDTH,
                                                               self.OFFSETS[1] + self.GRID_WIDTH * n)], 1)

    def drive(self, val):
        if self.direction == 0:
            newval = self.robotLoc[0] + val
            if newval >= 0 and newval <= 6:
                self.robotLoc[0] = newval
        elif self.direction == 2:
            newval = self.robotLoc[0] - val
            if newval >= 0 and newval <= 6:
                self.robotLoc[0] = newval
        elif self.direction == 1:
            newval = self.robotLoc[1] - val
            if newval >= 0 and newval <= 6:
                self.robotLoc[1] = newval
        elif self.direction == 3:
            newval = self.robotLoc[1] + val
            if newval >= 0 and newval <= 6:
                self.robotLoc[1] = newval
            # self.putRobotInBlock(self.make_location(self.robotLoc))

    def rotateCounterClockwise(self):
        self.direction = (self.direction + 1) % 4

    def rotateClockwise(self):
        self.direction = (self.direction - 1) % 4

    def handleMouseEvent(self, event):
        pass


class MapBlock():
    color_OT = (255, 0, 0)
    color_DEADEND = (0, 255, 255)
    color_START = (255, 255, 0)
    color_EMPTY = (0, 0, 0)

    def __init__(self, screen, gameboard, coords, grid_width, grid_key):
        self.screen = screen
        self.coords = coords
        self.GAMEBOARD = gameboard
        self.GRID_WIDTH = grid_width
        self.loc = grid_key
        self.object = pygame.Rect(self.coords, (self.GRID_WIDTH, self.GRID_WIDTH))
        self.color = self.color_EMPTY
        self.type = 'E'
        self.visited = False
        self.obstructed = False
        self.observed = False
        self.perimeter = False
        self.corner = False

    def draw(self):
        self.object.topleft = (self.GAMEBOARD.OFFSETS[0] + self.coords[0], self.GAMEBOARD.OFFSETS[1] + self.coords[1])
        self.screen.fill(self.color, self.object)

    def setStart(self):
        self.color = self.color_START
        self.type = 'E'

    def setOT(self):
        self.color = self.color_OT
        self.type = 'T'

    def setDeadend(self):
        self.color = self.color_DEADEND
        self.type = 'D'

    def setEmpty(self):
        self.color = self.color_EMPTY
        self.type = 'E'


from Sensors import Distance_Sensor, Electromagnetic_Sensor, Capacitive_Sensor
