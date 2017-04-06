import random
from collections import deque  # stack and queue
import heapq  # priority queue
import time
import sys

from Robot import Robot as SimRobot  # passed to simulation_impl

from Grid_Util import *
from Static_Decorator import static_vars
from ThresholdFinder import ThresholdFinder
from hampath import HamiltonianPath
from Timer import Timer

PRIORITY_FOR_AVOIDING_TURNS = 1

MOVE_COUNT_ALLOWED_AWAY_FROM_SIDES = 13

BAD_CALIBRATION_COORDINATES = set()
# If the 2x4s don't align well with each other on each side, uncomment the following lines
BAD_CALIBRATION_COORDINATES.add(Coordinate(GRID_WIDTH / 2, 0))
BAD_CALIBRATION_COORDINATES.add(Coordinate(0, GRID_HEIGHT / 2))
BAD_CALIBRATION_COORDINATES.add(Coordinate(GRID_WIDTH / 2, GRID_HEIGHT - 1))
BAD_CALIBRATION_COORDINATES.add(Coordinate(GRID_WIDTH - 1, GRID_HEIGHT / 2))
# print(BAD_CALIBRATION_COORDINATES)


def simulation_impl(_sim_parameters):
    """
    Pull simulation robot from simulation.
    :param _sim_parameters: [SimRobot]
    """
    _robot_interface = _sim_parameters[0]
    _sim_buttons = _sim_parameters[1]
    robot = Robot(_robot_interface, _sim_buttons)
    robot.explore3()


def translate_coordinate_to_index(coordinate):
    """
    change a coordinate to the index on the robot's map display
    index is 0 in top left, counting up to the right
    :param coordinate: Coordinate
    :return: index on the robot's map display
    :rtype: int
    """
    return ((DISPLAY_HEIGHT - 1) - coordinate.y) * DISPLAY_WIDTH + coordinate.x


def unit_tests_for_translate_coordinates_to_index():
    assert translate_coordinate_to_index(Coordinate(0, 6)) == 8
    assert translate_coordinate_to_index(Coordinate(0, 0)) == 56
    assert translate_coordinate_to_index(Coordinate(6, 1)) == 54
    assert translate_coordinate_to_index(Coordinate(2, 2)) == 42

# comment out this line for faster loading in deployment:
# unit_tests_for_translate_coordinates_to_index()


class GridSpaceData:
    """ information collected and learned about 1 space on the grid """
    def __init__(self):
        self.tunnelHere = Knowledge.unknown
        self.wireHere = Knowledge.unknown
        self.cacheHere = Knowledge.unknown
        self._obstacleHere = Knowledge.unknown

        self.tunnelReading = None
        self.wireReading = None

        self.visited = False

        # TODO: this cache is open, this cache has die

    def set_obstacle(self, see_obstacle):
        """
        :return: needs to be visited
        :rtype: bool
        """
        if see_obstacle:
            self._obstacleHere = Knowledge.yes
        else:
            self._obstacleHere = Knowledge.no
            if not self.visited:
                return True
        return False

    def get_obstacle_here(self):
        return self._obstacleHere


def cost_of_this_move(grid_space_data, coordinate):
    """ calculate the cost of 1 grid space move for Dijkstra's algorithm """
    cost = 5  # default
    if not grid_space_data.visited:
        cost -= 3
    if coordinate.x == 0 or coordinate.x == GRID_WIDTH - 1 or coordinate.y == 0 or coordinate.y == GRID_HEIGHT - 1:
        # edge
        cost -= 1
        if (coordinate.x == 0 or coordinate.x == GRID_WIDTH - 1) and \
                (coordinate.y == 0 or coordinate.y == GRID_HEIGHT - 1):
            # corner
            cost -= 1
    return cost


class HeapqItem:
    """ object that goes in the priority queue for Dijkstra's algorithm
        a coordinate, the directions of how to get there, and the cost of taking those directions """
    def __init__(self, _coordinate, _directions, _base_cost, current_facing_direction):
        self.coordinate = _coordinate
        self.directions = _directions
        self.base_cost = _base_cost  # base cost before taking turns into account
        self.cost = None
        self.calculate_cost(current_facing_direction)
        # sys.stdout.write("put path into priority q: to " + str(self.coordinate) +
        #                  " cost: " + str(self.cost) + " _base_cost: " + str(_base_cost) + '\n' +
        #                  "direction list: " + str(self.directions) + '\n')

    def calculate_cost(self, current_facing_direction):
        """ taking turns into account """
        self.cost = self.base_cost
        previous_direction = current_facing_direction
        for direction in self.directions:
            if direction != previous_direction:
                self.cost += PRIORITY_FOR_AVOIDING_TURNS
            previous_direction = direction

    def __lt__(self, other):
        return self.cost < other.cost

    def __gt__(self, other):
        return self.cost > other.cost

    def __le__(self, other):
        return self.cost <= other.cost

    def __ge__(self, other):
        return self.cost >= other.cost

    def __eq__(self, other):
        return self.cost == other.cost

    def __repr__(self):
        return str(self.coordinate)


def can_calibrate_x(coord):
    print("checking a coordinate to be able to calibrate on x")
    print(coord)
    print((coord.x == 0 or coord.x == GRID_WIDTH - 1) and coord not in BAD_CALIBRATION_COORDINATES)
    return (coord.x == 0 or coord.x == GRID_WIDTH - 1) and coord not in BAD_CALIBRATION_COORDINATES


class GridData:
    """ collection of all the GridSpaceData for the whole grid """
    def __init__(self):
        self.data = []
        self.needToVisit = set()
        self.canMoveHere = set()
        for x in range(DISPLAY_WIDTH):
            for y in range(DISPLAY_HEIGHT):
                self.data.append(GridSpaceData())

        self.set_begin_known_information()

    def get(self, x_or_coordinate, y=-1):
        """
        :return: the GridSpaceData at the given coordinate
        :rtype: GridSpaceData
        """
        if isinstance(x_or_coordinate, Coordinate):
            x = x_or_coordinate.x
            y = x_or_coordinate.y
        else:
            x = x_or_coordinate
        # invalid x and y checking could go here
        return self.data[x * DISPLAY_HEIGHT + y]

    def set_begin_known_information(self):
        # the information that we know before we start
        for x in range(GRID_WIDTH):
            for y in range(GRID_HEIGHT):
                if (x == 0 or x == GRID_WIDTH - 1) or (y == 0 or y == GRID_HEIGHT - 1):
                    # edge
                    if self.get(x, y).set_obstacle(Knowledge.no):
                        self.needToVisit.add(Coordinate(x, y))
                        self.canMoveHere.add(Coordinate(x, y))
                    if (x == 0 or x == GRID_WIDTH - 1) and (y == 0 or y == GRID_HEIGHT - 1):
                        # corner
                        self.get(x, y).tunnelHere = Knowledge.no
                        self.get(x, y).wireHere = Knowledge.no
                        self.get(x, y).cacheHere = Knowledge.no
                    else:
                        # edge not in corner
                        self.get(x, y).cacheHere = Knowledge.yes  # all edges not in corners have cache
                else:
                    # not on the edge
                    self.get(x, y).cacheHere = Knowledge.no

    def find_shortest_known_path(self, from_coordinate, to_coordinate, current_facing_direction):
        """
        find a path from one place to another with known information
        :param current_facing_direction:
        :param from_coordinate:
        :type from_coordinate: Coordinate
        :param to_coordinate: destination or "s" for finding a path to side
        :type to_coordinate: str | Coordinate
        :return: list of directions
        :rtype: list
        """
        # BFS / Dijkstra's
        visited_in_this_bfs = set()
        bfs_queue = []
        current_path = HeapqItem(from_coordinate, [], 0, current_facing_direction)
        # Coord, directions we took to get there, cost, current facing

        while (to_coordinate == "s" and not can_calibrate_x(current_path.coordinate)) or \
              (to_coordinate != "s" and current_path.coordinate != to_coordinate):
            visited_in_this_bfs.add(current_path.coordinate)

            for direction in COORDINATE_CHANGE:
                coord_checking = current_path.coordinate + COORDINATE_CHANGE[direction]
                if coord_checking in self.canMoveHere and coord_checking not in visited_in_this_bfs:
                    heapq.heappush(bfs_queue,
                                   HeapqItem(coord_checking,
                                             current_path.directions + [direction],
                                             current_path.base_cost + cost_of_this_move(self.get(coord_checking),
                                                                                        coord_checking),
                                             current_facing_direction))

            # get the new shortest path from priority queue
            print(bfs_queue)
            current_path = heapq.heappop(bfs_queue)

        return current_path.directions


class Robot:
    SLEEP_TIME = 0.1
    TIME_LIMIT = 350  # seconds (6 minutes minus some safety time)
    WEIGHT_FOR_TIME_AVERAGE = .9
    TEMP_WIRE_THRESHOLD = 65  # TODO: pick a good number threshold for EMF

    def __init__(self, outside_grid_or_robot_interface, outside_buttons=None):
        self.gridData = GridData()
        self.position = Coordinate()
        if isinstance(outside_grid_or_robot_interface, OutsideGrid):
            self.outside_grid = outside_grid_or_robot_interface
            self.using_outside_grid = True
        else:  # isinstance(outside_grid_or_robot_interface, SimRobot):
            self.robot_interface = outside_grid_or_robot_interface
            self.using_outside_grid = False
            self.sim_buttons = outside_buttons
        """
        else:
            raise TypeError("Invalid argument passed to constructor")
        """

        self.facing = Direction.east  # TODO: can we get this from the SimRobot?

        self.away_from_sides_count = 0

        # this is to correct calibration after seeing how forwards have been influenced by calibration
        self.forward_count = 0

        # end before 6 minutes
        self.timer = Timer()
        # weighted averages
        self.average_time_for_forward = 4
        self.average_time_for_turn = 2
        self.average_time_for_calibration = 2

    @staticmethod
    def wait_till_done(resp):
        intermediate_delay = 0.01
        while not resp.isDone:
            time.sleep(intermediate_delay)
        time.sleep(Robot.SLEEP_TIME)
        return resp.getResponse()

    def forward(self):
        # keep average forward time
        before_start = self.timer.get_elapsed_time()

        print("about to move forward from " + str(self.position))
        self.move_where_i_think_i_am(1)
        if self.using_outside_grid:
            self.display_grid_wait_enter()
        else:  # using interface
            self.wait_till_done(self.robot_interface.goForward())
            self.sleep_wait()
            self.display_grid_in_console()
        print("just moved to " + str(self.position))

        self.forward_count += 1

        # update average forward time
        self.average_time_for_forward = Robot.WEIGHT_FOR_TIME_AVERAGE * self.average_time_for_forward + \
            (1 - Robot.WEIGHT_FOR_TIME_AVERAGE) * (self.timer.get_elapsed_time() - before_start)
        print("average time for forward:", self.average_time_for_forward)

    def calibrate(self, need_to_be_facing=None):
        # keep average calibration time
        before_start = self.timer.get_elapsed_time()

        # figure out what side we can calibrate on
        can_calibrate_back = False
        can_calibrate_left = False
        can_calibrate_right = False

        direction_to_put_back_on_wood = None  # the robot needs to face so it can calibrate on back

        if self.position in BAD_CALIBRATION_COORDINATES:
            pass  # leave them all false

        elif self.facing == Direction.east:
            if self.position.x == 0:
                can_calibrate_back = True
            if self.position.y == 0:
                can_calibrate_right = True
                direction_to_put_back_on_wood = Direction.north
            elif self.position.y == GRID_HEIGHT - 1:
                can_calibrate_left = True
                direction_to_put_back_on_wood = Direction.south

        elif self.facing == Direction.north:
            if self.position.y == 0:
                can_calibrate_back = True
            if self.position.x == 0:
                can_calibrate_left = True
                direction_to_put_back_on_wood = Direction.east
            elif self.position.x == GRID_WIDTH - 1:
                can_calibrate_right = True
                direction_to_put_back_on_wood = Direction.west

        elif self.facing == Direction.west:
            if self.position.x == GRID_WIDTH - 1:
                can_calibrate_back = True
            if self.position.y == 0:
                can_calibrate_left = True
                direction_to_put_back_on_wood = Direction.north
            elif self.position.y == GRID_HEIGHT - 1:
                can_calibrate_right = True
                direction_to_put_back_on_wood = Direction.south

        elif self.facing == Direction.south:
            if self.position.y == GRID_HEIGHT - 1:
                can_calibrate_back = True
            if self.position.x == 0:
                can_calibrate_right = True
                direction_to_put_back_on_wood = Direction.east
            elif self.position.x == GRID_WIDTH - 1:
                can_calibrate_left = True
                direction_to_put_back_on_wood = Direction.west

        # now we know which sides we can calibrate on, so do it
        print("about to calibrate")
        if self.using_outside_grid:
            pass  # calibrate
        else:
            # TODO: in a corner, which calibration works better, back? or side?
            # do that one first (elif means only do one)
            if can_calibrate_back:
                self.wait_till_done(self.robot_interface.goCalibrateIR("B"))
            elif can_calibrate_left:
                # see if this is one of the special left calibrations where we can adjust the calibration
                if self.forward_count > 2:  # not one of the special ones
                    good_distance = self.wait_till_done(self.robot_interface.goCalibrateIR("L"))
                    if good_distance == '0':
                        self.turn(direction_to_put_back_on_wood)
                        self.wait_till_done(self.robot_interface.goCalibrateIR("B"))
                        # in case directions were interrupted
                        if need_to_be_facing is not None:
                            self.turn(need_to_be_facing)
                            self.wait_till_done(self.robot_interface.goCalibrateIR("L"))
                elif self.forward_count == 2:
                    # left calibration to correct left calibration
                    good_distance = self.wait_till_done(self.robot_interface.goCalibrateIR("l"))
                    if good_distance == '0':
                        self.turn(direction_to_put_back_on_wood)
                        self.wait_till_done(self.robot_interface.goCalibrateIR("B"))
                elif self.forward_count == 1:
                    # left calibration to correct back calibration
                    self.wait_till_done(self.robot_interface.goCalibrateIR("b"))

            elif can_calibrate_right:
                good_distance = self.wait_till_done(self.robot_interface.goCalibrateIR("R"))
                if good_distance == '0':
                    self.turn(direction_to_put_back_on_wood)
                    self.wait_till_done(self.robot_interface.goCalibrateIR("B"))
                    # in case directions were interrupted
                    if need_to_be_facing is not None:
                        self.turn(need_to_be_facing)
                        self.wait_till_done(self.robot_interface.goCalibrateIR("R"))
            Robot.sleep_wait()
        print("calibration done")

        # if calibrated, update average calibration time
        if can_calibrate_right or can_calibrate_left or can_calibrate_back:
            self.average_time_for_calibration = Robot.WEIGHT_FOR_TIME_AVERAGE * self.average_time_for_calibration + \
                (1 - Robot.WEIGHT_FOR_TIME_AVERAGE) * (self.timer.get_elapsed_time() - before_start)

    def reverse(self):
        # TODO: this hasn't been updated for a long time (because it's not used)
        self.move_where_i_think_i_am(-1)

    def move_where_i_think_i_am(self, move_amount):
        """ don't call this, call forward or reverse instead """
        if self.facing == Direction.east:
            self.position.x += move_amount
        elif self.facing == Direction.north:
            self.position.y += move_amount
        elif self.facing == Direction.west:
            self.position.x -= move_amount
        elif self.facing == Direction.south:
            self.position.y -= move_amount

    def turn(self, desired_direction):
        # keep average of turn times
        before_start = self.timer.get_elapsed_time()

        print("about to turn: " + str(desired_direction))
        if self.facing == desired_direction:
            pass  # don't evaluate elif expressions
        # not already facing the correct direction
        elif int(desired_direction) == (int(self.facing) + 1) % int(Direction.count):
            self.left()
        elif (int(desired_direction) + 1) % int(Direction.count) == int(self.facing):
            self.right()
        else:  # 180
            # TODO: alternate 2 lefts and 2 rights in case of inaccuracies
            self.left()
            time.sleep(.2)  # to not maintain too much speed from first turn to second
            self.left()
        print("just turned: " + str(self.facing))
        # recalibrate if possible
        if self.using_outside_grid:
            self.display_grid_wait_enter()
        else:  # simulation
            self.display_grid_in_console()

        # update average time
        self.average_time_for_turn = Robot.WEIGHT_FOR_TIME_AVERAGE * self.average_time_for_turn + \
            (1 - Robot.WEIGHT_FOR_TIME_AVERAGE) * (self.timer.get_elapsed_time() - before_start)

    def right(self):
        """ use turn """
        if not self.using_outside_grid:
            self.wait_till_done(self.robot_interface.rotateClockwise())
            Robot.sleep_wait()
        if self.facing == Direction.east:
            self.facing = Direction.south
        else:
            self.facing -= 1  # self.facing = Direction(self.facing - 1)

    def left(self):
        """ use turn """
        if not self.using_outside_grid:
            self.wait_till_done(self.robot_interface.rotateCounterClockwise())
            Robot.sleep_wait()
        if self.facing == Direction.south:
            self.facing = Direction.east
        else:
            self.facing += 1  # self.facing = Direction(self.facing + 1)

    def report(self):
        return "coordinates: (" + str(self.position.x) + ", " + str(self.position.y) + \
               ") facing: " + str(self.facing)[10:]

    @static_vars(ROBOT_SYMBOLS={
            Direction.north: "^",
            Direction.south: "v",
            Direction.west: "<",
            Direction.east: ">"
        })
    def display_grid_in_console(self):

        for y in range(GRID_HEIGHT - 1, -1, -1):
            for x in range(GRID_WIDTH):
                if x == self.position.x and y == self.position.y:
                    # display robot facing
                    sys.stdout.write(self.display_grid_in_console.ROBOT_SYMBOLS[self.facing] + " ")
                else:
                    if self.gridData.get(x, y).get_obstacle_here() == Knowledge.yes:
                        sys.stdout.write("X" + " ")
                    else:
                        if self.gridData.get(x, y).wireHere == Knowledge.yes:
                            sys.stdout.write("Z" + " ")
                        elif self.gridData.get(x, y).visited:
                            sys.stdout.write("@" + " ")
                        else:  # not visited
                            sys.stdout.write("O" + " ")
            sys.stdout.write("\n")  # new line

    def display_grid_wait_enter(self):
        self.display_grid_in_console()
        raw_input()

    def see_obstacle(self, direction):
        if self.using_outside_grid:
            coord_looking = self.position + COORDINATE_CHANGE[direction]
            return self.outside_grid.data[coord_looking.x * GRID_HEIGHT + coord_looking.y].obstacle_here
        else:  # robot interface
            # which sensor
            # 0 right, 1 front, 2 left, 3 back
            which_sensor = (((direction - self.facing) % 4) + 1) % 4  # do we need the middle mod 4?
            try:
                return int(self.wait_till_done(self.robot_interface.readSensor(1))[which_sensor])
            except ValueError:
                return 1  # if serial comm error, just say that there is an obstacle

    def visit(self):
        print("visiting: " + str(self.position))
        self.gridData.get(self.position).visited = True
        self.gridData.needToVisit.remove(self.position)

        # look 4 directions for obstacles
        for direction in COORDINATE_CHANGE:  # for each of the 4 directions
            coord_to_check = self.position + COORDINATE_CHANGE[direction]  # the coordinate in this direction
            if (0 < coord_to_check.x < GRID_WIDTH - 1) and \
                    (0 < coord_to_check.y < GRID_HEIGHT - 1) and \
                    self.gridData.get(coord_to_check).get_obstacle_here() == Knowledge.unknown:
                if self.gridData.get(coord_to_check).set_obstacle(self.see_obstacle(direction)):
                    self.gridData.canMoveHere.add(coord_to_check)
                    self.gridData.needToVisit.add(coord_to_check)

        # take readings of capacity + EMF
        if self.using_outside_grid:
            pass  # TODO:
        else:
            # change color to mark visited
            self.robot_interface.MAP.grid[self.robot_interface.MAP.robotLoc[0]][self.robot_interface.MAP.robotLoc[1]].color = (75, 75, 75)
            # TODO: That ^ is not very modular
            try:
                self.gridData.get(self.position).wireReading = int(self.wait_till_done(self.robot_interface.readSensor(2))[0])
            except ValueError:
                self.gridData.get(self.position).wireReading = Knowledge.unknown
            try:
                self.gridData.get(self.position).tunnelReading = int(self.wait_till_done(self.robot_interface.readSensor(3))[0])
            except ValueError:
                self.gridData.get(self.position).tunnelReading = Knowledge.unknown
            """ uncomment this to re-enable old sim map marking
            if self.gridData.get(self.position).wireReading == 1:
                self.robot_interface.MAP.markOT()
            elif self.gridData.get(self.position).tunnelReading == 1:
                self.robot_interface.MAP.markDeadend()
            """

            # temporary wire light in case we don't get to the end
            if self.gridData.get(self.position).wireReading > Robot.TEMP_WIRE_THRESHOLD:
                self.wait_till_done(self.robot_interface.set8x8(translate_coordinate_to_index(self.position), "T"))

    def explore(self):
        """ visit all possible grid spaces """
        # old one, probably missing something

        dfs_stack = deque()
        dfs_stack.append(self.position)
        while len(self.gridData.needToVisit):
            coord_at_top = dfs_stack[-1]
            if coord_at_top in self.gridData.needToVisit:
                # find directions
                directions = self.gridData.find_shortest_known_path(self.position, coord_at_top, self.facing)
                # go there
                for direction in directions:
                    self.turn(direction)
                    self.forward()
                    # visit everywhere along the way
                    if self.position in self.gridData.needToVisit:
                        self.visit()
                # in case there are no directions
                if self.position in self.gridData.needToVisit:
                    self.visit()
            # put adjacent unvisited nodes in stack and visit them
            # order: south north west east, to touch walls often and fill holes early
            found_adjacent_block_to_visit = False
            for direction in (Direction.south, Direction.north, Direction.west, Direction.east):
                coord_to_check = COORDINATE_CHANGE[direction] + coord_at_top
                if coord_to_check in self.gridData.needToVisit:
                    dfs_stack.append(coord_to_check)
                    found_adjacent_block_to_visit = True
                    break
            if found_adjacent_block_to_visit:
                continue  # visit it

            # if we arrive here, none of the adjacent blocks need to be visited
            dfs_stack.pop()

    def beginning_stuff(self):
        self.timer.start()  # so it's started if not using_outside_grid

        # perform if running a bot
        if not self.using_outside_grid:
            # light up yellow READY light on 8x8 (A7)
            self.wait_till_done(self.robot_interface.setReadyLight())
            # wait for Go Button to be pressed
            # """ comment this to wait for start button
            while not int(self.wait_till_done(self.sim_buttons.getGoButton())):
                time.sleep(0.25)
            # """

            # go button was just pushed, start timer
            self.timer.start()

            # make a guess on the seven segment for the die
            self.wait_till_done(self.robot_interface.set7segment(3))

            # beginning calibration
            self.wait_till_done(self.robot_interface.beginningObstacleThresholdCalibration())
            self.wait_till_done(self.robot_interface.beginningRightCalibration())
            self.wait_till_done(self.robot_interface.beginningBackCalibration())
            self.turn(Direction.north)
            self.wait_till_done(self.robot_interface.goCalibrateIR('B'))
            self.wait_till_done(self.robot_interface.beginningLeftCalibration())

    def explore3short(self):
        self.beginning_stuff()

        # forward, turn around, back to start
        self.forward()

        self.calibrate()

        self.turn(Direction.south)

        self.forward()

        self.turn(Direction.east)

        self.wait_till_done(self.robot_interface.goCalibrateIR("B"))

        self.turn(Direction.north)

        self.wait_till_done(self.robot_interface.goCalibrateIR("B"))

    def explore3(self):
        print("explore function has been called")
        """ visit all possible grid spaces
            go back to sides, when away for a long time """

        self.beginning_stuff()

        # ready to go
        keep_going = True
        self.away_from_sides_count = 0
        dfs_stack = deque()
        dfs_stack.append(Coordinate(self.position.x, self.position.y))
        print("about to enter main explore loop")
        while len(self.gridData.needToVisit) and self.time_left() and keep_going:
            print("elapsed time: " + str(self.timer.get_elapsed_time()))

            coord_at_top = dfs_stack[-1]
            if coord_at_top in self.gridData.needToVisit:
                # find directions
                directions = self.gridData.find_shortest_known_path(self.position, coord_at_top, self.facing)
                # go there
                keep_going = self.travel_these(directions, coord_at_top, dfs_stack)

            if self.away_from_sides_count > MOVE_COUNT_ALLOWED_AWAY_FROM_SIDES:
                print("I've been away too long...")
                directions = self.gridData.find_shortest_known_path(self.position, "s", self.facing)
                print("Here are the directions to a side I can calibrate on:")
                print(directions)
                # go there
                keep_going = self.travel_these(directions, coord_at_top, dfs_stack)
            # put adjacent unvisited nodes in stack and visit them
            coord_at_top = dfs_stack[-1]
            # order: south north west east, to touch walls often and fill holes early
            found_adjacent_block_to_visit = False
            for direction in (Direction.south, Direction.north, Direction.west, Direction.east):
                coord_to_check = COORDINATE_CHANGE[direction] + coord_at_top
                if coord_to_check in self.gridData.needToVisit:
                    print("appending: " + str(coord_to_check))
                    dfs_stack.append(coord_to_check)
                    found_adjacent_block_to_visit = True
                    break  # for loop
            if found_adjacent_block_to_visit:
                continue  # while loop, visit it

            # if we arrive here, none of the adjacent blocks need to be visited
            print("removing: " + str(dfs_stack[-1]))
            dfs_stack.pop()
            print(dfs_stack)

        print("out of main explore loop - number of places not visited: " + str(len(self.gridData.needToVisit)))

        readings_analyzed = False

        # if we didn't run out of time analyze readings now
        if not len(self.gridData.needToVisit):
            cache_locations = self.analyze_readings()
            readings_analyzed = True

            # TODO: look for dice in caches
            # find out which cache will be a shorter path
            paths_to_cache = [[] for i in range(len(cache_locations))]
            paths_to_start = [[] for i in range(len(cache_locations))]
            minimum_path_index = 0
            for index in range(len(cache_locations)):
                paths_to_cache[index] = self.gridData.find_shortest_known_path(self.position,
                                                                               cache_locations[index],
                                                                               self.facing)
                paths_to_start[index] = self.gridData.find_shortest_known_path(cache_locations[index],
                                                                               Coordinate(0, 0),
                                                                               paths_to_cache[index][-1])
                if len(paths_to_cache[index]) + len(paths_to_start[index]) < \
                   len(paths_to_cache[minimum_path_index]) + len(paths_to_start[minimum_path_index]):
                    minimum_path_index = index

            # paths...[minimum_path_index] are now the shortest path to cache and then to start

            if len(cache_locations):  # only if we have cache locations
                # see if we have time to travel to this closest cache
                estimated_move_time = (self.average_time_for_calibration +
                                       self.average_time_for_turn +
                                       self.average_time_for_forward)
                if self.timer.get_elapsed_time() + (estimated_move_time * (len(paths_to_cache[minimum_path_index]) +
                                                                           len(paths_to_start[minimum_path_index]) +
                                                                           2)) > \
                        Robot.TIME_LIMIT:  # + 2 for extra time it takes to lift lid
                    # we have enough time
                    # TODO: go to cache and look at die
                    # self.travel_these(paths_to_cache[minimum_path_index], None, None, False)
                    # lift lid
                    # look at die
                    # set number on seven segment
                    pass

        # find directions to start
        directions = self.gridData.find_shortest_known_path(self.position, Coordinate(0, 0), self.facing)

        # in case we stopped because of the time limit,
        # remove everything that's not in these final directions from need_to_visit,
        # and mark them as obstacles
        if len(self.gridData.needToVisit):
            coordinates_in_final_path = []
            current_position_in_this_traversal = self.position
            for direction in directions:
                current_position_in_this_traversal += COORDINATE_CHANGE[direction]
                coordinates_in_final_path.append(current_position_in_this_traversal)
            need_to_visit_list = list(self.gridData.needToVisit)
            for coordinate in need_to_visit_list:
                if coordinate not in coordinates_in_final_path:
                    self.gridData.needToVisit.remove(coordinate)
                    self.gridData.get(coordinate).set_obstacle(True)

        # go there
        self.travel_these(directions, None, None, True)
        print("done traveling back to start: " + str(self.position))

        # analyze readings to find tunnel and wire
        if not readings_analyzed:
            self.analyze_readings()

        # turn robot's back to field side and calibrate on it (to make sure we are fully in the starting square)
        if self.facing == Direction.south:
            self.turn(Direction.east)
        elif self.facing == Direction.west:
            self.turn(Direction.north)
        self.calibrate()

    @staticmethod
    def sleep_wait():
        time.sleep(Robot.SLEEP_TIME)

    def travel_these(self, directions, coord_at_top, dfs_stack, visit_and_explore_along_the_way=True):
        """
        travel the given set of directions
        :param directions:
        :param coord_at_top: of stack before anything changed this cycle
        :param dfs_stack:
        :param visit_and_explore_along_the_way:
        :return: keep going, the stop button has not been pressed
        :rtype: bool
        """
        for direction in directions:
            # stop algorithm if stop button is pressed
            if (not self.using_outside_grid) and int(self.wait_till_done(self.sim_buttons.getStopButton())):
                print("stop button pressed")
                return False

            self.turn(direction)
            self.calibrate(direction)
            self.forward()
            if len(directions) > 1:
                self.calibrate(direction)
            else:
                self.calibrate()

            if visit_and_explore_along_the_way:
                # count if away from sides
                if 0 < self.position.x < GRID_WIDTH - 1:
                    self.away_from_sides_count += 1
                else:
                    self.away_from_sides_count = 0
                print "away from side count: " + str(self.away_from_sides_count)

                # visit everywhere along the way
                if self.position in self.gridData.needToVisit:
                    self.visit()
                    if self.position != coord_at_top and dfs_stack is not None:
                        print("visiting somewhere along the way")
                        print("appending: " + str(self.position))
                        dfs_stack.append(Coordinate(self.position.x, self.position.y))
                        print(dfs_stack)

        if visit_and_explore_along_the_way:
            # in case there are no directions
            if self.position in self.gridData.needToVisit:
                self.visit()

        return True

    def analyze_readings(self):
        """
        analyze readings to find where wire and tunnel are
        :return: edge coordinates with wire
        :rtype: list
        """
        print("starting to analyze readings")

        # find thresholds
        # > threshold is yes
        # <= threshold is no (not >)
        algorithm_to_find_thresholds = ThresholdFinder(self.gridData.data)
        algorithm_to_find_thresholds.find_thresholds()
        list_of_wire_thresholds = algorithm_to_find_thresholds.get_wire_thresholds()
        list_of_tunnel_thresholds = algorithm_to_find_thresholds.get_tunnel_thresholds()

        # in case sensors malfunction and we get the same reading for everything
        if len(list_of_wire_thresholds) == 0:
            print("WARNING: no variation in wire sensor readings")
            list_of_wire_thresholds.append(0)
        if len(list_of_tunnel_thresholds) == 0:
            print("WARNING: no variation in foam sensor readings")
            list_of_tunnel_thresholds.append(0)

        # iterate through wire thresholds until we find one that doesn't fail
        # TODO: how to fail dead end tunnel thresholds?
        # (right now it just assumes that the first tunnel threshold won't fail)
        found_good_threshold = False
        force_using_this_threshold = False  # if all fail, use the first one
        wire_index = 0
        tunnel_index = 0
        edge_coordinates_with_wire = []

        while not found_good_threshold:
            wire_threshold = list_of_wire_thresholds[wire_index]
            tunnel_threshold = list_of_tunnel_thresholds[tunnel_index]

            print("trying threshold %d" % wire_threshold)

            reset_data = []
            # if this threshold fails reset these spaces to these values
            # tuple of (original wireHere, original tunnelHere)

            # apply thresholds to knowledge
            for space in self.gridData.data:
                reset_data.append((space.wireHere, space.tunnelHere))
                # tunnel first because wire/OT is subset of tunnel
                if space.tunnelReading is not None:
                    if space.tunnelReading > tunnel_threshold:
                        space.tunnelHere = Knowledge.yes
                    else:  # not > threshold
                        space.tunnelHere = Knowledge.no
                if space.wireReading is not None:
                    try:
                        if space.wireReading > wire_threshold or \
                                (space.cacheHere == Knowledge.yes and
                                 space.wireReading > 2.0 * wire_threshold / 3.0):
                            space.wireHere = Knowledge.yes
                        else:  # not > threshold
                            space.wireHere = Knowledge.no
                    except:
                        space.wireHere = Knowledge.no

            # make sure this didn't give us wire in the corners
            if (not force_using_this_threshold) and \
                    (self.gridData.get(0, 0).wireHere == Knowledge.yes or
                     self.gridData.get(0, GRID_HEIGHT-1).wireHere == Knowledge.yes or
                     self.gridData.get(GRID_WIDTH-1, 0).wireHere == Knowledge.yes or
                     self.gridData.get(GRID_WIDTH-1, GRID_HEIGHT-1).wireHere == Knowledge.yes):

                wire_index, force_using_this_threshold, cont = self.fail_threshold("puts wire in the corner",
                                                                                   wire_index,
                                                                                   list_of_wire_thresholds,
                                                                                   force_using_this_threshold,
                                                                                   reset_data)
                if cont:
                    continue

            print("here's the wire with that threshold")
            self.display_grid_in_console()

            # figure out wire(OT) under obstacles
            edge_coordinates_with_wire = []
            # top and bottom edges
            for x in range(1, GRID_WIDTH - 1):
                if self.gridData.get(x, 0).wireHere == Knowledge.yes:
                    edge_coordinates_with_wire.append(Coordinate(x, 0))
                if self.gridData.get(x, GRID_HEIGHT-1).wireHere == Knowledge.yes:
                    edge_coordinates_with_wire.append(Coordinate(x, GRID_HEIGHT-1))
            # left and right edges
            for y in range(1, GRID_HEIGHT - 1):
                if self.gridData.get(0, y).wireHere == Knowledge.yes:
                    edge_coordinates_with_wire.append(Coordinate(0, y))
                if self.gridData.get(GRID_WIDTH - 1, y).wireHere == Knowledge.yes:
                    edge_coordinates_with_wire.append(Coordinate(GRID_WIDTH - 1, y))
            # make sure there are exactly 2 edge coordinates with wire
            if (not force_using_this_threshold) and (len(edge_coordinates_with_wire) != 2):
                wire_index, force_using_this_threshold, cont = self.fail_threshold("not 2 edge spaces with wire",
                                                                                   wire_index,
                                                                                   list_of_wire_thresholds,
                                                                                   force_using_this_threshold,
                                                                                   reset_data)
                if cont:
                    continue
            # mark inside spaces adjacent to these edges as wire/OT
            # then leave in scope to prepare for following wire
            inner_coordinates = []
            directions_to_inner = []
            for each_space in edge_coordinates_with_wire:
                if each_space.x == 0:
                    inner_coordinates.append(Coordinate(1, each_space.y))
                    directions_to_inner.append(Direction.east)
                elif each_space.x == GRID_WIDTH-1:
                    inner_coordinates.append(Coordinate(GRID_WIDTH-2, each_space.y))
                    directions_to_inner.append(Direction.west)
                elif each_space.y == 0:
                    inner_coordinates.append(Coordinate(each_space.x, 1))
                    directions_to_inner.append(Direction.north)
                elif each_space.y == GRID_HEIGHT-1:
                    inner_coordinates.append(Coordinate(each_space.x, GRID_HEIGHT-2))
                    directions_to_inner.append(Direction.south)
                self.gridData.get(inner_coordinates[-1]).wireHere = Knowledge.yes

            # to make sure all the yes are included on the path, we need to count the yes
            # (only counting inner 25)
            yes_count = 0
            for x in range(1, GRID_WIDTH-1):
                for y in range(1, GRID_HEIGHT-1):
                    if self.gridData.get(x, y).wireHere == Knowledge.yes:
                        yes_count += 1

            max_turns = 3  # TODO: based on round (round 1: 2, round 2-3: 3)

            if len(inner_coordinates) > 1:
                algorithm_to_find_path = HamiltonianPath(self.gridData,
                                                         inner_coordinates[1],
                                                         inner_coordinates[0],
                                                         directions_to_inner[1],
                                                         Direction.opposite(directions_to_inner[0]),
                                                         yes_count,
                                                         max_turns)
                found_path_under_obstacles = algorithm_to_find_path.find_path()
            else:  # not enough edge coordinates with wire found
                found_path_under_obstacles = []
            # put path knowledge under obstacles
            for coordinate in found_path_under_obstacles:
                self.gridData.get(coordinate).wireHere = Knowledge.yes
            if (not force_using_this_threshold) and (len(found_path_under_obstacles) == 0):
                wire_index, force_using_this_threshold, cont = self.fail_threshold("couldn't find path under obstacles",
                                                                                   wire_index,
                                                                                   list_of_wire_thresholds,
                                                                                   force_using_this_threshold,
                                                                                   reset_data)
                if cont:
                    continue
            found_good_threshold = True

        if not self.using_outside_grid:
            print("sending information to display now")
            # send information to 8x8 display
            for row in range(GRID_HEIGHT):
                for col in range(GRID_WIDTH):
                    if self.gridData.get(col, row).wireHere == Knowledge.yes:
                        self.wait_till_done(self.robot_interface.set8x8(
                            translate_coordinate_to_index(Coordinate(col, row)),
                            "T"))
                    elif self.gridData.get(col, row).tunnelHere == Knowledge.yes:
                        self.wait_till_done(self.robot_interface.set8x8(
                            translate_coordinate_to_index(Coordinate(col, row)),
                            "D"))
                    elif col == 0 and row == 0:
                        self.wait_till_done(self.robot_interface.setReadyLight())
                    else:
                        self.wait_till_done(self.robot_interface.set8x8(
                            translate_coordinate_to_index(Coordinate(col, row)),
                            "E"))
        return edge_coordinates_with_wire

    def fail_threshold(self, reason, wire_index, list_of_wire_thresholds, force_using_this_threshold, reset_data):
        """
        this threshold doesn't create a correct board configuration
        :param reason: what failed
        :type reason: str
        :param wire_index:
        :type wire_index: int
        :param list_of_wire_thresholds:
        :type list_of_wire_thresholds: list
        :param force_using_this_threshold:
        :type force_using_this_threshold: bool
        :param reset_data:
        :type reset_data: list
        :return: wire_index, force_using_this_threshold, continue
        :rtype: tuple
        """

        # TODO: to test this function, we need invalid board configurations
        # like wire in the corner, or more than two side blocks with wire,
        # or wire that doesn't go all the way from one end to the other
        #
        # so this functionality hasn't really been tested at all yet

        index = wire_index
        force = force_using_this_threshold
        cont = False
        print("wire threshold index " + str(wire_index) + " " + reason)
        if wire_index == len(list_of_wire_thresholds) - 1:  # this is the last threshold there is to try
            if wire_index == 0:  # there is only one threshold to try
                force = True
            else:  # there were earlier thresholds that failed
                # use the first one even though it fails
                self.reset_knowledge(reset_data)
                index = 0
                force = True
                cont = True
        else:  # there are more thresholds to try
            # try the next threshold
            self.reset_knowledge(reset_data)
            index = wire_index + 1
            cont = True

        return index, force, cont

    def reset_knowledge(self, reset_data):
        index = 0
        for space in self.gridData.data:
            space.wireHere = reset_data[index][0]
            space.tunnelHere = reset_data[index][1]
            index += 1

    def time_left(self):
        directions_to_start = self.gridData .find_shortest_known_path(self.position, Coordinate(0, 0), self.facing)
        estimated_time = (self.average_time_for_forward +
                          self.average_time_for_turn +
                          self.average_time_for_calibration) * len(directions_to_start)

        return self.timer.get_elapsed_time() + estimated_time < Robot.TIME_LIMIT


class OutsideGrid:
    class GridSpace:
        def __init__(self, _obstacle):
            self.obstacle_here = _obstacle
            # TODO: add tunnel, wire, cache stuff

    def __init__(self):
        self.data = []
        for x in range(GRID_WIDTH):
            for y in range(GRID_HEIGHT):
                self.data.append(OutsideGrid.GridSpace(False))

    def input_from_console(self):
        print("give me Os and Xs")
        for y in range(GRID_HEIGHT - 1, -1, -1):
            row = ""
            while len(row) != GRID_WIDTH:
                row = raw_input("row " + str(y) + ": ")
            for x in range(GRID_WIDTH):
                self.data[x * GRID_HEIGHT + y].obstacle_here = (row[x] == "X")

    def random_obstacles(self):
        for y in range(1, GRID_HEIGHT - 1):
            for x in range(1, GRID_WIDTH - 1):
                self.data[x * GRID_HEIGHT + y].obstacle_here = random.choice((True, False))


def test():
    outside_grid = OutsideGrid()

    answer = ""
    while not answer:
        answer = raw_input("(n)o obstacles or (r)andom obstacles or (t)ype in obstacles? ")
        if answer:
            answer = answer[0].lower()
            if answer == "n":
                break
            elif answer == "r":
                outside_grid.random_obstacles()
                break
            elif answer == "t":
                outside_grid.input_from_console()
                break
            answer = ""
    robot = Robot(outside_grid)
    print(robot.report())
    robot.explore3()


def main():
    test()

if __name__ == "__main__":
    main()

# TODO: debug this with explore3:
# OOOOOOO
# OXXXXOO
# OOOXXXO
# OXXOOOO
# OOXOXOO
# OXOOXXO
# OOOOOOO

# need to put every visit in queue

# TODO: interrupt path to recalibrate
"""
@ @ @ @ @ @ @
@ @ @ @ @ X @
@ X @ X X @ @
@ > X X @ @ @
@ X @ * @ @ @
@ X X X @ @ @
@ @ @ @ @ @ @
"""
