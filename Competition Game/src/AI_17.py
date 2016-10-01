import random
from collections import deque
from enum import IntEnum
import heapq
from Robot import Robot as SimRobot

import sys

GRID_WIDTH = 7
GRID_HEIGHT = 7
DISPLAY_WIDTH = 8
DISPLAY_HEIGHT = 8


def simulation_impl(sim_robot):
    pass


class Coordinate:
    def __init__(self, _x=0, _y=0):
        self.x = _x
        self.y = _y

    def __eq__(self, other):
        return self.x == other.x and self.y == other.y

    def __ne__(self, other):
        return not self.__eq__(other)

    def __hash__(self):
        return self.x * DISPLAY_WIDTH + self.y

    def __add__(self, other):
        return Coordinate(self.x + other.x, self.y + other.y)

    def __repr__(self):
        return "(" + str(self.x) + ", " + str(self.y) + ")"


SNAKE_AROUND_EDGE_SEQUENCE = [
    Coordinate(0, 0),
    Coordinate(0, 1),
    Coordinate(0, 2),
    Coordinate(1, 2),
    Coordinate(1, 3),
    Coordinate(0, 3),
    Coordinate(0, 4),
    Coordinate(1, 4),
    Coordinate(1, 5),
    Coordinate(0, 5),
    Coordinate(0, 6),
    Coordinate(1, 6),
    Coordinate(2, 6),
    Coordinate(2, 5),
    Coordinate(2, 4),
    Coordinate(3, 4),
    Coordinate(3, 5),
    Coordinate(3, 6),
    Coordinate(4, 6),
    Coordinate(4, 5),
    Coordinate(4, 4),
    Coordinate(5, 4),
    Coordinate(5, 5),
    Coordinate(5, 6),
    Coordinate(6, 6),
    Coordinate(6, 5),
    Coordinate(6, 4),
    Coordinate(6, 3),
    Coordinate(5, 3),
    Coordinate(4, 3),
    Coordinate(4, 2),
    Coordinate(5, 2),
    Coordinate(6, 2),
    Coordinate(6, 1),
    Coordinate(6, 0),
    Coordinate(5, 0),
    Coordinate(5, 1),
    Coordinate(4, 1),
    Coordinate(4, 0),
    Coordinate(3, 0),
    Coordinate(3, 1),
    Coordinate(3, 2),
    Coordinate(3, 3),
    Coordinate(2, 3),
    Coordinate(2, 2),
    Coordinate(2, 1),
    Coordinate(2, 0),
    Coordinate(1, 0),
    Coordinate(1, 1)
]


class Knowledge(IntEnum):
    unknown = -1
    yes = 1
    no = 0


class Direction(IntEnum):
    east = 0
    north = 1
    west = 2
    south = 3
    count = 4

COORDINATE_CHANGE = {
    Direction.east: Coordinate(1, 0),
    Direction.west: Coordinate(-1, 0),
    Direction.north: Coordinate(0, 1),
    Direction.south: Coordinate(0, -1)
}


class GridSpaceData:
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
    def __init__(self, _coordinate, _directions, _cost):
        self.coordinate = _coordinate
        self.directions = _directions
        self.cost = _cost

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
    def __init__(self):
        self.data = []
        self.needToVisit = set()
        self.canMoveHere = set()
        for x in range(DISPLAY_WIDTH):
            for y in range(DISPLAY_HEIGHT):
                self.data.append(GridSpaceData())

        self.set_begin_known_information()

    def get(self, x_or_coordinate, y=-1):
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

    def find_shortest_known_path(self, from_coordinate, to_coordinate):

        # BFS
        visited_in_this_bfs = set()
        bfs_queue = []
        current_path = HeapqItem(from_coordinate, [], 0)  # Coord, directions we took to get there, cost

        while current_path.coordinate != to_coordinate:
            visited_in_this_bfs.add(current_path.coordinate)

            for direction in COORDINATE_CHANGE:
                coord_checking = current_path.coordinate + COORDINATE_CHANGE[direction]
                if coord_checking in self.canMoveHere and coord_checking not in visited_in_this_bfs:
                    heapq.heappush(bfs_queue,
                                   HeapqItem(coord_checking,
                                             current_path.directions + [direction],
                                             current_path.cost + cost_of_this_move(self.get(coord_checking),
                                                                                   coord_checking)))

            # get the new shortest path from priority queue
            print(bfs_queue)
            current_path = heapq.heappop(bfs_queue)

        return current_path.directions

    def find_path_to_side(self, from_coordinate):
        # BFS
        visited_in_this_bfs = set()
        bfs_queue = []
        current_path = HeapqItem(from_coordinate, [], 0)  # Coord, directions we took to get there, cost

        while 0 < current_path.coordinate.x < GRID_WIDTH - 1:
            visited_in_this_bfs.add(current_path.coordinate)

            for direction in COORDINATE_CHANGE:
                coord_checking = current_path.coordinate + COORDINATE_CHANGE[direction]
                if coord_checking in self.canMoveHere and coord_checking not in visited_in_this_bfs:
                    heapq.heappush(bfs_queue,
                                   HeapqItem(coord_checking,
                                             current_path.directions + [direction],
                                             current_path.cost + cost_of_this_move(self.get(coord_checking),
                                                                                   coord_checking)))

            # get the new shortest path from priority queue
            current_path = heapq.heappop(bfs_queue)

        return current_path.directions


class Robot:
    def __init__(self, _outside_grid):
        self.gridData = GridData()
        self.position = Coordinate()
        self.facing = Direction.north
        self.outside_grid = _outside_grid

    def forward(self):
        self.move(1)
        self.display_grid_wait_enter()

    def reverse(self):
        self.move(-1)

    def move(self, move_amount):
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
        self.display_grid_wait_enter()
        return

    def right(self):
        """ use turn """
        if self.facing == Direction.east:
            self.facing = Direction.south
        else:
            self.facing = Direction(self.facing - 1)

    def left(self):
        """ use turn """
        if self.facing == Direction.south:
            self.facing = Direction.east
        else:
            self.facing = Direction(self.facing + 1)

    def report(self):
        return "coordinates: (" + str(self.position.x) + ", " + str(self.position.y) + \
               ") facing: " + str(self.facing)[10:]

    def display_grid_in_console(self):
        ROBOT_SYMBOLS = {
            Direction.north: "^",
            Direction.south: "v",
            Direction.west: "<",
            Direction.east: ">"
        }

        for y in range(GRID_HEIGHT - 1, -1, -1):
            for x in range(GRID_WIDTH):
                if x == self.position.x and y == self.position.y:
                    # display robot facing
                    sys.stdout.write(ROBOT_SYMBOLS[self.facing] + " ")
                else:
                    if self.gridData.get(x, y).get_obstacle_here() == Knowledge.yes:
                        sys.stdout.write("X" + " ")
                    else:
                        if self.gridData.get(x, y).visited:
                            sys.stdout.write("@" + " ")
                        else:  # not visited
                            sys.stdout.write("O" + " ")
            print()  # new line

    def display_grid_wait_enter(self):
        self.display_grid_in_console()
        raw_input()

    def see_obstacle(self, direction):
        # TODO: replace this with readings from sensors
        coord_looking = self.position + COORDINATE_CHANGE[direction]
        return self.outside_grid.data[coord_looking.x * GRID_HEIGHT + coord_looking.y].obstacle_here

    def visit(self):
        self.gridData.get(self.position).visited = True
        self.gridData.needToVisit.remove(self.position)

        # look 4 directions for obstacles  TODO: change this to use COORDINATE_CHANGE?
        # west
        coord_to_check = Coordinate(self.position.x - 1, self.position.y)
        if self.position.x and self.gridData.get(coord_to_check).get_obstacle_here() == Knowledge.unknown:
            if self.gridData.get(coord_to_check).set_obstacle(self.see_obstacle(Direction.west)):
                self.gridData.canMoveHere.add(coord_to_check)
                self.gridData.needToVisit.add(coord_to_check)
        # east
        coord_to_check = Coordinate(self.position.x + 1, self.position.y)
        if self.position.x < GRID_WIDTH - 1 and \
                self.gridData.get(coord_to_check).get_obstacle_here() == Knowledge.unknown:
            if self.gridData.get(coord_to_check).set_obstacle(self.see_obstacle(Direction.east)):
                self.gridData.canMoveHere.add(coord_to_check)
                self.gridData.needToVisit.add(coord_to_check)
        # north
        coord_to_check = Coordinate(self.position.x, self.position.y + 1)
        if self.position.y < GRID_HEIGHT - 1 and \
                self.gridData.get(coord_to_check).get_obstacle_here() == Knowledge.unknown:
            if self.gridData.get(coord_to_check).set_obstacle(self.see_obstacle(Direction.north)):
                self.gridData.canMoveHere.add(coord_to_check)
                self.gridData.needToVisit.add(coord_to_check)
        # south
        coord_to_check = Coordinate(self.position.x, self.position.y - 1)
        if self.position.y and self.gridData.get(coord_to_check).get_obstacle_here() == Knowledge.unknown:
            if self.gridData.get(coord_to_check).set_obstacle(self.see_obstacle(Direction.south)):
                self.gridData.canMoveHere.add(coord_to_check)
                self.gridData.needToVisit.add(coord_to_check)

        # TODO: take readings and look for surrounding obstacles

    def explore(self):
        """ visit all possible grid spaces"""

        dfs_stack = deque()
        dfs_stack.append(self.position)
        while len(self.gridData.needToVisit):
            coord_at_top = dfs_stack[-1]
            if coord_at_top in self.gridData.needToVisit:
                # find directions
                directions = self.gridData.find_shortest_known_path(self.position, coord_at_top)
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
            # south
            coord_to_check = Coordinate(coord_at_top.x, coord_at_top.y - 1)
            if coord_to_check in self.gridData.needToVisit:
                dfs_stack.append(coord_to_check)
                continue
            # north
            coord_to_check = Coordinate(coord_at_top.x, coord_at_top.y + 1)
            if coord_to_check in self.gridData.needToVisit:
                dfs_stack.append(coord_to_check)
                continue
            # west
            coord_to_check = Coordinate(coord_at_top.x - 1, coord_at_top.y)
            if coord_to_check in self.gridData.needToVisit:
                dfs_stack.append(coord_to_check)
                continue
            # east
            coord_to_check = Coordinate(coord_at_top.x + 1, coord_at_top.y)
            if coord_to_check in self.gridData.needToVisit:
                dfs_stack.append(coord_to_check)
                continue

            # if we arrive here, none of the adjacent blocks need to be visited
            dfs_stack.pop()

    def explore_snake(self):
        """ this doesn't work """

        index = 0
        while len(self.gridData.needToVisit):
            if self.position in self.gridData.needToVisit:
                self.visit()
            coord_at_top = SNAKE_AROUND_EDGE_SEQUENCE[index]
            if coord_at_top in self.gridData.needToVisit:
                # find directions
                directions = self.gridData.find_shortest_known_path(self.position, coord_at_top)
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

            index += 1

    def explore3(self):
        """ visit all possible grid spaces
            go back to sides, when away for a long time """

        away_from_sides_count = 0
        dfs_stack = deque()
        dfs_stack.append(Coordinate(self.position.x, self.position.y))
        while len(self.gridData.needToVisit):
            coord_at_top = dfs_stack[-1]
            if coord_at_top in self.gridData.needToVisit:
                # find directions
                directions = self.gridData.find_shortest_known_path(self.position, coord_at_top)
                # go there
                for direction in directions:
                    self.turn(direction)
                    self.forward()
                    # count if away from sides
                    if 0 < self.position.x < GRID_WIDTH - 1:
                        away_from_sides_count += 1
                    else:
                        away_from_sides_count = 0
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

            if away_from_sides_count > 13:
                directions = self.gridData.find_path_to_side(self.position)
                # go there
                for direction in directions:
                    self.turn(direction)
                    self.forward()
                    # visit everywhere along the way
                    if self.position in self.gridData.needToVisit:
                        self.visit()
                        if self.position != coord_at_top:
                            print("appending: " + str(self.position))
                            dfs_stack.append(Coordinate(self.position.x, self.position.y))

            # put adjacent unvisited nodes in stack and visit them
            coord_at_top = dfs_stack[-1]
            # TODO: change this to use COORDINATE_CHANGE? (keep order)
            # order: south north west east, to touch walls often and fill holes early
            # south
            coord_to_check = Coordinate(coord_at_top.x, coord_at_top.y - 1)
            if coord_to_check in self.gridData.needToVisit:
                print("appending: " + str(coord_to_check))
                dfs_stack.append(coord_to_check)
                continue
            # north
            coord_to_check = Coordinate(coord_at_top.x, coord_at_top.y + 1)
            if coord_to_check in self.gridData.needToVisit:
                print("appending: " + str(coord_to_check))
                dfs_stack.append(coord_to_check)
                continue
            # west
            coord_to_check = Coordinate(coord_at_top.x - 1, coord_at_top.y)
            if coord_to_check in self.gridData.needToVisit:
                print("appending: " + str(coord_to_check))
                dfs_stack.append(coord_to_check)
                continue
            # east
            coord_to_check = Coordinate(coord_at_top.x + 1, coord_at_top.y)
            if coord_to_check in self.gridData.needToVisit:
                print("appending: " + str(coord_to_check))
                dfs_stack.append(coord_to_check)
                continue

            # if we arrive here, none of the adjacent blocks need to be visited
            print("removing: " + str(dfs_stack[-1]))
            dfs_stack.pop()
            print(dfs_stack)


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
    outside_grid.random_obstacles()
    #outside_grid.input_from_console()
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
