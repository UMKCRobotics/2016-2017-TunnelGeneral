import pygame, sys, os, random, math
from ast import literal_eval as make_tuple

__location__ = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))  # directory from which this script is ran
sys.path.insert(0, os.path.join(__location__))

from Stage import Stage
from MouseEvents import MouseEvents


class Stage_Build(Stage):
    def __init__(self, screen, saved=None):
        Stage.__init__(self, screen)
        temprect = pygame.Rect((0, 0), (1220, 800))
        self.robot_creator = Robot_Creator(screen, temprect.center)
        self.focused = None
        self.global_objects.append(self.robot_creator)
        self.mouse_objects = []
        self.mouse = MouseEvents(self.mouse_objects)
        self.mouse_objects.append(Distance_Sensor_Creator(self.screen, self.mouse, self, (100, 100)))
        self.mouse_objects.append(Electromagnetic_Sensor_Creator(self.screen, self.mouse, self, (100, 150)))
        self.mouse_objects.append(Capacitive_Sensor_Creator(self.screen, self.mouse, self, (100, 200)))
        if saved != None:
            listLoad = self.load_robot(saved)
            #print listLoad
            self.mouse_objects.extend(listLoad)
        self.global_objects.extend(self.mouse_objects)
        #print self.global_objects
        self.color_HIGHLIGHT = (255, 255, 0)

    def remove_focused(self):
        self.mouse_objects.remove(self.focused)
        self.global_objects.remove(self.focused)
        self.focused = None

    def handleEvents(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return self.signal_QUIT()
            if event.type == pygame.KEYDOWN:
                if self.focused:
                    self.focused.handleKeyEvent(event)
                if event.key in [pygame.K_ESCAPE]:
                    return self.signal_QUIT()
                if event.key in [pygame.K_DELETE]:
                    if self.focused:
                        self.remove_focused()
                if event.key in [pygame.K_c]:
                    return self.signal_NEXT_STAGE((self.robot_creator, self.generate_sensor_list()))
                if event.key in [pygame.K_p]:
                    self.clean_up_sensors()
                if event.key in [pygame.K_s]:
                    self.save_robot()
            if event.type in [pygame.MOUSEBUTTONDOWN, pygame.MOUSEBUTTONUP]:
                self.mouse.handleMouseEvent(event)
        return self.signal_NO_ACTION()

    def performAllStageActions(self):
        returnVal = self.handleEvents()
        self.mouse.performActions()
        self.screen.fill((220, 220, 220))
        for obj in self.global_objects:
            obj.draw()
        if self.focused is not None:
            pygame.draw.rect(self.screen, self.color_HIGHLIGHT,
                             (self.focused.object.x, self.focused.object.y, self.focused.object.w,
                              self.focused.object.h), 3)
        return returnVal

    def clean_up_sensors(self):
        for item in self.mouse_objects:
            if isinstance(item, Sensor_Template):
                if not self.robot_creator.object.contains(item.object):
                    self.mouse_objects.remove(item)
                    self.global_objects.remove(item)

    def generate_sensor_list(self):
        self.clean_up_sensors()
        temp_list = []
        for item in self.mouse_objects:
            if isinstance(item, Sensor_Template):
                temp_list.append(item)
        return temp_list

    def save_robot(self):
        self.clean_up_sensors()
        # choose filename
        counter = 0
        filename = None
        robotpath = os.path.join(__location__, 'robots')
        files = os.listdir(robotpath)
        while True:
            filename = 'robot' + str(counter) + '.txt'
            if filename not in files:
                break
            else:
                counter += 1

        sensor_list = self.generate_sensor_list()
        with open(os.path.join(robotpath, filename), 'wb') as robotfile:
            # save offsets,size,color of sensor
            for sensor in sensor_list:
                robotfile.write(sensor.type + ':' + str(sensor.OFFSETS) + ':' + str(
                    (sensor.TOTAL_WIDTH, sensor.TOTAL_HEIGHT)) + ':' + str(sensor.color) + ':' + str(
                    sensor.direction) + '\n')
        print 'robot saved under %s' % filename

    def load_robot(self, filename):
        robotpath = os.path.join(__location__, 'robots')
        files = os.listdir(robotpath)
        #print files
        if not filename.endswith('.txt'):
            filename += '.txt'
        if filename not in files:
            print 'robot %s cannot be found; load aborted' % filename
            return []
        else:
            print 'trying to load robot'
            with open(os.path.join(robotpath, filename), 'rb') as robotfile:
                sensor_list = []
                for line in robotfile:
                    data = line.strip().split(':')
                    if len(data) != 5:
                        print 'sensor could not be loaded; not enough data'
                        continue
                    data_type = data[0]
                    offsets = make_tuple(data[1])
                    size = make_tuple(data[2])
                    color = make_tuple(data[3])
                    direction = int(data[4])
                    newSensor = None
                    if data_type == 'Distance':
                        newSensor = Distance_Sensor_Template(self.screen, self, offsets, size, color)
                    elif data_type == 'Electromagnetic':
                        newSensor = Electromagnetic_Sensor_Template(self.screen, self, offsets, size, color)
                    elif data_type == 'Capacitive':
                        newSensor = Capacitive_Sensor_Template(self.screen, self, offsets, size, color)
                    if newSensor is not None:
                        newSensor.direction = direction
                        sensor_list.append(newSensor)
                        # sensor_list.reverse()
            for item in sensor_list:
                #print item.type
                pass
            return sensor_list


class Robot_Creator():
    def __init__(self, screen, center):
        self.screen = screen
        self.TOTAL_WIDTH = 300
        self.TOTAL_HEIGHT = 300
        self.object = pygame.Rect((0, 0), (self.TOTAL_WIDTH, self.TOTAL_HEIGHT))
        self.object.center = center
        self.OFFSETS = self.object.topleft
        self.OBJECTS = []
        self.color_ROBOT = (26, 148, 49)
        self.text_color = (0, 0, 0)
        self.text_content = "FRONT"
        self.font = pygame.font.Font(None, 24)

    def draw(self):
        self.object.topleft = self.OFFSETS
        self.screen.fill(self.color_ROBOT, self.object)
        self.text = self.font.render(self.text_content, 1, self.text_color)
        textpos = self.text.get_rect()
        textpos.midleft = self.object.midright
        self.screen.blit(self.text, textpos)
        for sensor in self.OBJECTS:
            sensor.draw()

    def handleMouseEvent(self, event):
        pass


class Sensor_Creator():
    def __init__(self, screen, mouse, stage, offsets):
        self.OFFSETS = offsets
        self.stage = stage
        self.screen = screen
        self.mouse = mouse
        self.TOTAL_WIDTH = 0
        self.TOTAL_HEIGHT = 0
        self.object = None
        self.color = (0, 0, 0)
        self.class_to_create = None
        self.text_color = (0, 0, 0)
        self.text_content = "Sensor"
        self.font = pygame.font.Font(None, 16)
        self.type = None

    def draw(self):
        self.object.topleft = self.OFFSETS
        self.screen.fill(self.color, self.object)
        # draw text
        self.text = self.font.render(self.text_content, 1, self.text_color)
        textpos = self.text.get_rect()
        textpos.center = self.object.center
        self.screen.blit(self.text, textpos)

    def handleMouseEvent(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN:
            #print self.class_to_create
            self.stage.focused = self.class_to_create(self.screen, self.stage, self.OFFSETS,
                                                      (self.TOTAL_WIDTH, self.TOTAL_HEIGHT), self.color)
            self.stage.global_objects.append(self.stage.focused)
            self.stage.mouse_objects.append(self.stage.focused)
            self.mouse.curObject = self.stage.focused


class Distance_Sensor_Creator(Sensor_Creator):
    def __init__(self, screen, mouse, stage, offsets):
        Sensor_Creator.__init__(self, screen, mouse, stage, offsets)
        self.TOTAL_WIDTH = 30
        self.TOTAL_HEIGHT = 30
        self.object = pygame.Rect(self.OFFSETS, (self.TOTAL_WIDTH, self.TOTAL_HEIGHT))
        self.direction = 0
        self.color = (255, 0, 255)
        self.class_to_create = Distance_Sensor_Template
        self.text_content = "Distance"
        self.type = "Distance_Creator"


class Electromagnetic_Sensor_Creator(Sensor_Creator):
    def __init__(self, screen, mouse, stage, offsets):
        Sensor_Creator.__init__(self, screen, mouse, stage, offsets)
        self.TOTAL_WIDTH = 40
        self.TOTAL_HEIGHT = 40
        self.object = pygame.Rect(self.OFFSETS, (self.TOTAL_WIDTH, self.TOTAL_HEIGHT))
        self.direction = 0
        self.color = (255, 122, 0)
        self.class_to_create = Electromagnetic_Sensor_Template
        self.text_content = "Electromagnetic"
        self.type = "Electromagnetic_Creator"


class Capacitive_Sensor_Creator(Sensor_Creator):
    def __init__(self, screen, mouse, stage, offsets):
        Sensor_Creator.__init__(self, screen, mouse, stage, offsets)
        self.TOTAL_WIDTH = 50
        self.TOTAL_HEIGHT = 50
        self.object = pygame.Rect(self.OFFSETS, (self.TOTAL_WIDTH, self.TOTAL_HEIGHT))
        self.direction = 0
        self.color = (122, 25, 25)
        self.class_to_create = Capacitive_Sensor_Template
        self.text_content = "Capacitive"
        self.type = "Capacitive_Creator"


class Sensor_Template():
    def __init__(self, screen, stage, offsets, size, color):
        self.screen = screen
        self.stage = stage
        self.OFFSETS = offsets
        self.TOTAL_WIDTH = size[0]
        self.TOTAL_HEIGHT = size[1]
        self.direction = 0
        self.object = None
        self.color = color
        self.color_direction = (0, 0, 0)
        self.type = None

    def draw(self):
        self.object.topleft = self.OFFSETS
        self.screen.fill(self.color, self.object)
        if self.direction == 0:
            dirRectangle = pygame.Rect(self.OFFSETS, (30, 10))
            dirRectangle.midleft = self.object.midright
            self.screen.fill(self.color_direction, dirRectangle)
        elif self.direction == 1:
            dirRectangle = pygame.Rect(self.OFFSETS, (10, 30))
            dirRectangle.midbottom = self.object.midtop
            self.screen.fill(self.color_direction, dirRectangle)
        elif self.direction == 2:
            dirRectangle = pygame.Rect(self.OFFSETS, (30, 10))
            dirRectangle.midright = self.object.midleft
            self.screen.fill(self.color_direction, dirRectangle)
        elif self.direction == 3:
            dirRectangle = pygame.Rect(self.OFFSETS, (10, 30))
            dirRectangle.midtop = self.object.midbottom
            self.screen.fill(self.color_direction, dirRectangle)

    def rotateCounterClockwise(self):
        self.changeDirection(1)

    def rotateClockwise(self):
        self.changeDirection(-1)

    def changeDirection(self, val):
        self.direction = (self.direction + val) % 4
        #print self.direction

    def handleKeyEvent(self, event):
        if event.key in [pygame.K_a, pygame.K_LEFT]:
            self.rotateCounterClockwise()
        elif event.key in [pygame.K_d, pygame.K_RIGHT]:
            self.rotateClockwise()

    def handleMouseEvent(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN:
            self.stage.focused = self


class Distance_Sensor_Template(Sensor_Template):
    def __init__(self, screen, stage, offsets, size, color):
        Sensor_Template.__init__(self, screen, stage, offsets, size, color)
        self.type = "Distance"
        self.object = pygame.Rect(self.OFFSETS, (self.TOTAL_WIDTH, self.TOTAL_HEIGHT))


class Electromagnetic_Sensor_Template(Sensor_Template):
    def __init__(self, screen, stage, offsets, size, color):
        Sensor_Template.__init__(self, screen, stage, offsets, size, color)
        self.type = "Electromagnetic"
        self.object = pygame.Rect(self.OFFSETS, (self.TOTAL_WIDTH, self.TOTAL_HEIGHT))


class Capacitive_Sensor_Template(Sensor_Template):
    def __init__(self, screen, stage, offsets, size, color):
        Sensor_Template.__init__(self, screen, stage, offsets, size, color)
        self.type = "Capacitive"
        self.object = pygame.Rect(self.OFFSETS, (self.TOTAL_WIDTH, self.TOTAL_HEIGHT))
