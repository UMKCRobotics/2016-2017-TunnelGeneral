import pygame, sys, os, random, math

__location__ = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))  # directory from which this script is ran
sys.path.insert(0, os.path.join(__location__))

from Robot import Robot
from Stage_Build import Robot_Creator


class Sensor_Converter():
    def __init__(self, robot, robotCr, sensor_templates):
        self.robot = robot
        self.robotCr = robotCr
        self.sensor_templates = sensor_templates

    def create_robot_sensors(self):
        for sensor_temp in self.sensor_templates:
            sensor = self.convertSensor(sensor_temp)
            if sensor != None:
                self.robot.sensors.append(sensor)

    def convertSensor(self, sensor_temp):  # returns converted sensor
        created_sensor = None
        robot = self.robot
        robotCr = self.robotCr
        xNew = (sensor_temp.object.x - robotCr.object.x) * (float(robot.object.w) / robotCr.object.w)
        yNew = (sensor_temp.object.y - robotCr.object.y) * (float(robot.object.h) / robotCr.object.h)
        wNew = (sensor_temp.object.w * (float(robot.object.w) / robotCr.object.w))
        hNew = (sensor_temp.object.h * (float(robot.object.h) / robotCr.object.h))
        if sensor_temp.type == 'Distance':
            created_sensor = Distance_Sensor(self.robot, (xNew, yNew), (wNew, hNew), sensor_temp.color,
                                             sensor_temp.direction)
        elif sensor_temp.type == 'Electromagnetic':
            created_sensor = Electromagnetic_Sensor(self.robot, (xNew, yNew), (wNew, hNew), sensor_temp.color,
                                                    sensor_temp.direction)
        elif sensor_temp.type == 'Capacitive':
            created_sensor = Capacitive_Sensor(self.robot, (xNew, yNew), (wNew, hNew), sensor_temp.color,
                                               sensor_temp.direction)

        return created_sensor


class Sensor():
    def __init__(self, robot, coords, size, color, direction):
        self.robot = robot
        self.screen = self.robot.screen
        self.coords = coords
        self.size = size
        self.color = color
        self.object = pygame.Rect(coords, size)
        self.read_obj = pygame.Rect((0, 0), (0, 0))
        self.read_obj_size = (0, 0)
        self.direction = direction

    def initialize_read_obj(self):
        pass

    def draw(self):
        # rel_direction = (self.direction+self.robot.direction)%4
        rel_direction = self.robot.direction
        if rel_direction == 0:
            self.object.size = (self.size[0], self.size[1])
            self.object.topleft = (
            self.coords[0] + self.robot.object.topleft[0], self.coords[1] + self.robot.object.topleft[1])
        elif rel_direction == 1:
            self.object.size = (self.size[1], self.size[0])
            self.object.bottomleft = (
            self.robot.object.bottomleft[0] + self.coords[1], self.robot.object.bottomleft[1] - self.coords[0])
        elif rel_direction == 2:
            self.object.size = (self.size[0], self.size[1])
            self.object.bottomright = (
            self.robot.object.bottomright[0] - self.coords[0], self.robot.object.bottomright[1] - self.coords[1])
        elif rel_direction == 3:
            self.object.size = (self.size[1], self.size[0])
            self.object.topright = (
            self.robot.object.topright[0] - self.coords[1], self.robot.object.topright[1] + self.coords[0])
        self.screen.fill(self.color, self.object)

    def read_sensor(self):
        pass


class Distance_Sensor(Sensor):
    def __init__(self, robot, coords, size, color, direction):
        Sensor.__init__(self, robot, coords, size, color, direction)
        self.read_obj_size = (self.robot.GRID_WIDTH, self.object.h)
        self.initialize_read_obj()

    def initialize_read_obj(self):
        self.orient_read_obj()

    def orient_read_obj(self):
        rel_direction = (self.direction + self.robot.direction) % 4
        if rel_direction == 0:
            self.read_obj.size = (self.read_obj_size[0], self.read_obj_size[1])
            self.read_obj.midleft = self.object.midright
        elif rel_direction == 1:
            self.read_obj.size = (self.read_obj_size[1], self.read_obj_size[0])
            self.read_obj.midbottom = self.object.midtop
        elif rel_direction == 2:
            self.read_obj.size = (self.read_obj_size[0], self.read_obj_size[1])
            self.read_obj.midright = self.object.midleft
        elif rel_direction == 3:
            self.read_obj.size = (self.read_obj_size[1], self.read_obj_size[0])
            self.read_obj.midtop = self.object.midbottom

    def read_sensor(self, environment_objects):
        self.orient_read_obj()
        val = self.read_obj.collidelist(environment_objects)
        if val < 0:
            return 0
        else:
            return 1


class Electromagnetic_Sensor(Sensor):
    def __init__(self, robot, coords, size, color, direction):
        Sensor.__init__(self, robot, coords, size, color, direction)
        self.read_obj_size = (self.object.size[0] * 2, self.object.size[1] * 2)
        self.initialize_read_obj()

    def initialize_read_obj(self):
        self.orient_read_obj()

    def orient_read_obj(self):
        self.read_obj.size = self.read_obj_size
        self.read_obj.center = self.object.center

    def read_sensor(self, environment_objects):
        self.orient_read_obj()
        val = self.read_obj.collidelist(environment_objects)
        if val < 0:
            return 0
        else:
            return 1


class Capacitive_Sensor(Sensor):
    def __init__(self, robot, coords, size, color, direction):
        Sensor.__init__(self, robot, coords, size, color, direction)
        self.read_obj_size = (self.object.size[0] * 2, self.object.size[1] * 2)
        self.initialize_read_obj()

    def initialize_read_obj(self):
        self.orient_read_obj()

    def orient_read_obj(self):
        self.read_obj.size = self.read_obj_size
        self.read_obj.center = self.object.center

    def read_sensor(self, environment_objects):
        self.orient_read_obj()
        val = self.read_obj.collidelist(environment_objects)
        if val < 0:
            return 0
        else:
            return 1
