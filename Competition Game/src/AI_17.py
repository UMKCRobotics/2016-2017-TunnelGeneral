import random
from collections import deque  # stack and queue
import heapq  # priority queue
from Robot import Robot as SimRobot  # passed to simulation_impl
import time
import sys
from Grid_Util import *
from Static_Decorator import static_vars

PRIORITY_FOR_AVOIDING_TURNS = 1

MOVE_COUNT_ALLOWED_AWAY_FROM_SIDES = 13

GRID_WIDTH = 7
GRID_HEIGHT = 7
DISPLAY_WIDTH = 8
DISPLAY_HEIGHT = 8


def simulation_impl(_sim_parameters):
    """
    Pull simulation robot from simulation.
    :param _sim_parameters: [SimRobot]
    :return:
    """
    _sim_robot = _sim_parameters[0]
    _sim_buttons = _sim_parameters[1]
    robot = Robot(_sim_robot, _sim_buttons)
    robot.explore3()


# this is an enumeration, but we can't use enumerations because someone doesn't update their python
class Knowledge:  # class Knowledge(IntEnum):
    def __init__(self):
        pass

    unknown = -1
    yes = 1
    no = 0


def translate_coordinate_to_index(coordinate):
    """
    change a coordinate to the index on the display
    index is 0 in top left, counting up to the right
    :param coordinate: Coordinate
    :return: int
    """
    return ((DISPLAY_HEIGHT - 1) - coordinate.y) * DISPLAY_WIDTH + coordinate.x


def unit_tests_for_translate_coordinates_to_index():
    assert translate_coordinate_to_index(Coordinate(0, 6)) == 8
    assert translate_coordinate_to_index(Coordinate(0, 0)) == 56
    assert translate_coordinate_to_index(Coordinate(6, 1)) == 54
    assert translate_coordinate_to_index(Coordinate(2, 2)) == 42

# TODO: comment out this line for faster loading in deployment:
unit_tests_for_translate_coordinates_to_index()


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
        """ :return needs to be visited """
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
        """ :return GridSpaceData """
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

        :param current_facing_direction:
        :param from_coordinate:
        :param to_coordinate:
        :return list: list of directions
        """
        # BFS / Dijkstra's
        visited_in_this_bfs = set()
        bfs_queue = []
        current_path = HeapqItem(from_coordinate, [], 0, current_facing_direction)
        # Coord, directions we took to get there, cost, current facing

        while current_path.coordinate != to_coordinate:
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

    def find_path_to_side(self, from_coordinate, current_facing_direction):
        # BFS  TODO: undo copied code
        visited_in_this_bfs = set()
        bfs_queue = []
        current_path = HeapqItem(from_coordinate, [], 0, current_facing_direction)
        # Coord, directions we took to get there, cost, current facing

        while 0 < current_path.coordinate.x < GRID_WIDTH - 1:
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
            current_path = heapq.heappop(bfs_queue)

        return current_path.directions


class Robot:
    SLEEP_TIME = 0.1

    def __init__(self, outside_grid_or_sim_robot_interface, outside_buttons=None):
        self.gridData = GridData()
        self.position = Coordinate()
        if isinstance(outside_grid_or_sim_robot_interface, OutsideGrid):
            self.outside_grid = outside_grid_or_sim_robot_interface
            self.using_outside_grid = True
        elif isinstance(outside_grid_or_sim_robot_interface, SimRobot):
            self.sim_robot = outside_grid_or_sim_robot_interface
            self.using_outside_grid = False
            self.sim_buttons = outside_buttons
        else:
            raise TypeError("Invalid argument passed to constructor")

        self.facing = Direction.east  # TODO: can we get this from the SimRobot?

    @staticmethod
    def wait_till_done(resp):
        intermediate_delay = 0.01
        while not resp.isDone:
            time.sleep(intermediate_delay)
        return resp.getResponse()

    def forward(self):
        self.move_where_i_think_i_am(1)
        if self.using_outside_grid:
            self.display_grid_wait_enter()
        else:  # using simulation
            self.wait_till_done(self.sim_robot.goForward())
            self.sleep_wait()
            self.display_grid_in_console()

    def calibrate(self):  # alias for going forward (for sim)
        if self.using_outside_grid:
            pass  # TODO: calibrate
        else:
            self.wait_till_done(self.sim_robot.goForward())
            Robot.sleep_wait()

    def reverse(self):
        # TODO: this hasn't been updated for simulation (because it's not used)
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
            self.left()
        # recalibrate if possible
        if self.using_outside_grid:
            self.display_grid_wait_enter()
        else:  # simulation
            self.display_grid_in_console()
        return

    def right(self):
        """ use turn """
        if not self.using_outside_grid:
            self.wait_till_done(self.sim_robot.rotateClockwise())
            Robot.sleep_wait()
        if self.facing == Direction.east:
            self.facing = Direction.south
        else:
            self.facing -= 1  # self.facing = Direction(self.facing - 1)

    def left(self):
        """ use turn """
        if not self.using_outside_grid:
            self.wait_till_done(self.sim_robot.rotateCounterClockwise())
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
                        if self.gridData.get(x, y).visited:
                            sys.stdout.write("@" + " ")
                        else:  # not visited
                            sys.stdout.write("O" + " ")
            sys.stdout.write("\n")  # new line

    def display_grid_wait_enter(self):
        self.display_grid_in_console()
        raw_input()

    def see_obstacle(self, direction):
        # TODO: replace this with readings from sensors
        if self.using_outside_grid:
            coord_looking = self.position + COORDINATE_CHANGE[direction]
            return self.outside_grid.data[coord_looking.x * GRID_HEIGHT + coord_looking.y].obstacle_here
        else:  # simulation
            # which sensor
            # 0 right, 1 front, 2 left, 3 back
            which_sensor = (((direction - self.facing) % 4) + 1) % 4  # do we need the middle mod 4?
            return self.wait_till_done(self.sim_robot.readSensor(1))[which_sensor]

    def visit(self):
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

        # TODO: take readings
        # take readings of capacity + EMF
        if self.using_outside_grid:
            pass  # TODO:
        else:
            # change color to mark visited
            self.sim_robot.MAP.grid[self.sim_robot.MAP.robotLoc[0]][self.sim_robot.MAP.robotLoc[1]].color = (75, 75, 75)
            # TODO: That ^ is not very modular
            if self.wait_till_done(self.sim_robot.readSensor(2))[0] == 1:
                self.sim_robot.MAP.markOT()
            elif self.wait_till_done(self.sim_robot.readSensor(3))[0] == 1:
                self.sim_robot.MAP.markDeadend()

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
            # TODO: change this to use COORDINATE_CHANGE? (keep order)
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

    def explore3(self):
        """ visit all possible grid spaces
            go back to sides, when away for a long time """

        # wait for Go Button to be pressed
        if not self.using_outside_grid:
            while not int(self.wait_till_done(self.sim_buttons.getGoButton())):
                time.sleep(0.1)

        keep_going = True
        away_from_sides_count = 0
        dfs_stack = deque()
        dfs_stack.append(Coordinate(self.position.x, self.position.y))
        while len(self.gridData.needToVisit) and keep_going:
            coord_at_top = dfs_stack[-1]
            if coord_at_top in self.gridData.needToVisit:
                # find directions
                directions = self.gridData.find_shortest_known_path(self.position, coord_at_top, self.facing)
                # go there
                for direction in directions:
                    # stop algorithm if stop button is pressed
                    if int(self.wait_till_done(self.sim_buttons.getStopButton())):
                        keep_going = False
                        break

                    self.turn(direction)
                    self.forward()

                    # count if away from sides
                    if 0 < self.position.x < GRID_WIDTH - 1:
                        away_from_sides_count += 1
                    else:
                        away_from_sides_count = 0
                    print "away from side count: " + str(away_from_sides_count)

                    # visit everywhere along the way
                    if self.position in self.gridData.needToVisit:
                        self.visit()
                        if self.position != coord_at_top:
                            print("visiting somewhere along the way")
                            print("appending: " + str(self.position))
                            dfs_stack.append(Coordinate(self.position.x, self.position.y))
                            print(dfs_stack)
                # in case there are no directions
                if self.position in self.gridData.needToVisit:
                    self.visit()

            if away_from_sides_count > MOVE_COUNT_ALLOWED_AWAY_FROM_SIDES:
                directions = self.gridData.find_path_to_side(self.position, self.facing)
                # go there
                for direction in directions:
                    # stop algorithm if stop button is pressed
                    if int(self.wait_till_done(self.sim_buttons.getStopButton())):
                        keep_going = False
                        break

                    self.turn(direction)
                    self.forward()

                    # count if away from sides
                    if 0 < self.position.x < GRID_WIDTH - 1:
                        away_from_sides_count += 1
                    else:
                        away_from_sides_count = 0
                    print "away from side count: " + str(away_from_sides_count)

                    # visit everywhere along the way
                    if self.position in self.gridData.needToVisit:
                        self.visit()
                        if self.position != coord_at_top:
                            print("appending: " + str(self.position))
                            dfs_stack.append(Coordinate(self.position.x, self.position.y))
                self.calibrate()  # TODO: put this at every edge
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

        # TODO: look for dice in caches
        # go back to start
        directions = self.gridData.find_shortest_known_path(self.position, Coordinate(0, 0), self.facing)
        # go there
        for direction in directions:
            if int(self.wait_till_done(self.sim_buttons.getStopButton())):
                break
            self.turn(direction)
            self.forward()

    @staticmethod
    def sleep_wait():
        time.sleep(Robot.SLEEP_TIME)


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
