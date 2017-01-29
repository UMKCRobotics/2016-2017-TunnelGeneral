from Grid_Util import *

ASSUME_PATH_CANT_GO_ADJACENT_TO_ITSELF = True


# adapted from http://www.geeksforgeeks.org/backtracking-set-7-hamiltonian-cycle/

class HamiltonianPath:
    def __init__(self, _grid_data, _start, _end, _first_direction, _last_direction, _yes_count, _max_turns):
        """
        find path in middle 25
        :param _grid_data: robot.gridData
        :param _start: 1 away from the edge
        :param _end: 1 away from the edge
        :param _yes_count: not counting the 2 edges
        """
        self.gridData = _grid_data
        self.path = [Coordinate(_start.x, _start.y)]
        self.directions = [_first_direction]
        self.last_direction = _last_direction
        self.end = Coordinate(_end.x, _end.y)
        self.yes_count = _yes_count
        self.max_turns = _max_turns

    def is_safe(self, coordinate_to_add, direction_to_add):
        print("checking safety of:")
        print(str(self.path) + str(coordinate_to_add))
        # don't go to the edges
        if (coordinate_to_add.x == 0 or
                coordinate_to_add.x == GRID_WIDTH-1 or
                coordinate_to_add.y == 0 or
                coordinate_to_add.y == GRID_HEIGHT-1):
            print("bad: it went to an edge")
            return False

        # don't go to a no
        if self.gridData.get(coordinate_to_add).wireHere == Knowledge.no:
            print("bad: went to a no")
            return False

        if ASSUME_PATH_CANT_GO_ADJACENT_TO_ITSELF:
            path_index = len(self.path) - 2  # second to last step in path
            while path_index >= 0:
                for direction in COORDINATE_CHANGE:
                    if self.path[path_index] == coordinate_to_add + COORDINATE_CHANGE[direction]:
                        print("bad: it went adjacent to itself")
                        return False
                path_index -= 1

        # check if we can move here from previous step in path
        """ I don't think this is necessary.
        # restrict from moving from unknown to yes if yes can come from somewhere else
        if self.gridData.get(coordinate_to_add).wireHere == Knowledge.yes:
            if self.gridData.get(self.path[-1]).wireHere == Knowledge.unknown:
                # moving from unknown to yes
                adjacent_yes_count = 0
                for direction in COORDINATE_CHANGE:
                    coordinate_checking = coordinate_to_add + COORDINATE_CHANGE[direction]
                    if self.gridData.get(coordinate_checking).wireHere == Knowledge.yes:
                        adjacent_yes_count += 1
                if adjacent_yes_count > 1:
                    return False
        el"""
        # restrict from moving from yes to unknown if yes can go somewhere else
        # (only if assuming the path can't go adjacent to itself)
        # this one might not be necessary either, I don't know
        if ASSUME_PATH_CANT_GO_ADJACENT_TO_ITSELF and \
                self.gridData.get(coordinate_to_add).wireHere == Knowledge.unknown:
            if self.gridData.get(self.path[-1]).wireHere == Knowledge.yes:
                # moving from yes to unknown
                adjacent_yes_count = 0
                for direction in COORDINATE_CHANGE:
                    coordinate_checking = self.path[-1] + COORDINATE_CHANGE[direction]
                    if self.gridData.get(coordinate_checking).wireHere == Knowledge.yes:
                        adjacent_yes_count += 1
                if adjacent_yes_count > 1:
                    print("bad: moved from yes to unknown when could go to another yes")
                    return False

        # check if we've already been here
        for coordinate in self.path:
            if coordinate == coordinate_to_add:
                print("bad: went someplace we've already been")
                return False

        # not more than 2/3 turns
        # print("counting turns in")
        # print(str(self.directions) + " " + str(direction_to_add) + " " + str(self.last_direction))
        turn_count = 0
        direction_index = 1
        while direction_index < len(self.directions):
            if self.directions[direction_index] != self.directions[direction_index - 1]:
                turn_count += 1
            direction_index += 1
        if direction_to_add != self.directions[len(self.directions) - 1]:
            turn_count += 1
        if self.last_direction != direction_to_add:
            turn_count += 1
        # print("counted turns: " + str(turn_count))
        if turn_count > self.max_turns:
            print("bad: too many turns in path")
            return False

        print("This one valid so far.")
        return True

    def ham_path_util(self):
        """ recursive """
        print("  recursive function path and directions:")
        print(self.path)
        print(self.directions)
        # base case: have a valid path
        if self.path[-1] == self.end:
            yes_in_path_count = 0
            for coordinate in self.path:
                if self.gridData.get(coordinate).wireHere == Knowledge.yes:
                    yes_in_path_count += 1
            return yes_in_path_count == self.yes_count

        # try adjacent yes and unknown as next candidate in path
        for direction in COORDINATE_CHANGE:
            candidate_coordinate = self.path[-1] + COORDINATE_CHANGE[direction]
            if self.is_safe(candidate_coordinate, direction):
                self.path.append(candidate_coordinate)
                self.directions.append(direction)

                # recur to construct the rest of the path
                if self.ham_path_util():
                    return True

                # if adding that didn't lead to a solution, then remove it
                self.path.pop()
                self.directions.pop()
        return False

    def find_path(self):
        if not self.ham_path_util():
            print("didn't find a path")
            return []
        else:
            print("found path:")
            print(self.path)
            return self.path
