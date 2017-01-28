# Utility classes for using a grid

GRID_WIDTH = 7
GRID_HEIGHT = 7
DISPLAY_WIDTH = 8
DISPLAY_HEIGHT = 8


# this is an enumeration, but we can't use enumerations because someone doesn't update their python
class Knowledge:  # class Knowledge(IntEnum):
    def __init__(self):
        pass

    unknown = -1
    yes = 1
    no = 0


class Coordinate:
    def __init__(self, _x=0, _y=0):
        self.x = _x
        self.y = _y

    def __eq__(self, other):
        return isinstance(other, Coordinate) and self.x == other.x and self.y == other.y

    def __ne__(self, other):
        return not self.__eq__(other)

    def __hash__(self):
        return self.x * DISPLAY_WIDTH + self.y

    def __add__(self, other):
        return Coordinate(self.x + other.x, self.y + other.y)

    def __repr__(self):
        return "(" + str(self.x) + ", " + str(self.y) + ")"


# this is an enumeration, but we can't use enumerations because someone doesn't update their python
class Direction:  # class Direction(IntEnum):
    def __init__(self):
        pass

    east = 0
    north = 1
    west = 2
    south = 3
    count = 4

    @staticmethod
    def opposite(given):
        return (given + 2) % 4


# To get the next coordinate in a given direction, add this coordinate
COORDINATE_CHANGE = {
    Direction.east: Coordinate(1, 0),
    Direction.west: Coordinate(-1, 0),
    Direction.north: Coordinate(0, 1),
    Direction.south: Coordinate(0, -1)
}
