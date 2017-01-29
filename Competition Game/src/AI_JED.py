from Robot import Robot
from time import sleep
import os

__location__ = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))  # directory from which this script is ran


def simulation_impl(sim_parameters):
    sim_robot = sim_parameters[0]
    sim_buttons = sim_parameters[1]
    robot = RobotAlg(sim_robot,sim_buttons)
    robot.doStuff()

""" Use the one in Robot.translate_coordinate_to_index
def translate_coordinate_to_index(coord):
    ""
    change a coordinate to the index on the 8x8 display
    index is 0 in top left, counting up to the right

    :param coord: a coordinate using this robot brain's system of coordinates
    :return: int - the index to be shown on the display
    ""
    x = coord[0]
    y = coord[1]

    index = y * 8 + x + 8

    return index
"""


class RobotAlg():
    SLEEP_TIME = 0.1

    def __init__(self, sim_robot, sim_buttons):
        self.sim_robot = sim_robot
        self.sim_buttons = sim_buttons
        self.MAP = self.sim_robot.MAP
        self.moves_since_cal = [0, 0]
        self.max_moves = [15, 15]
        self.goList = None
        self.DEBUG_MODE = False

    def wait_till_done(self,resp):
        intermediateDelay = 0.01
        while not resp.isDone:
            sleep(intermediateDelay)
        return resp.getResponse()

    def turn(self, desired_dir):  # turn robot to desired direction
        if desired_dir == self.MAP.direction:
            pass
        elif self.MAP.direction - desired_dir == 3 or self.MAP.direction - desired_dir == -1:
            self.wait_till_done(self.sim_robot.rotateCounterClockwise())
            sleep(self.SLEEP_TIME)
        elif self.MAP.direction - desired_dir == -3 or self.MAP.direction - desired_dir == 1:
            self.wait_till_done(self.sim_robot.rotateClockwise())
            sleep(self.SLEEP_TIME)
        else:  # turn twice to turn around
            self.wait_till_done(self.sim_robot.rotateCounterClockwise())
            sleep(self.SLEEP_TIME)
            self.wait_till_done(self.sim_robot.rotateCounterClockwise())
            sleep(self.SLEEP_TIME)

    def get_turn_amount(self, desired_dir):
        if desired_dir == self.MAP.direction:
            return 0
        elif self.MAP.direction - desired_dir == 3 or self.MAP.direction - desired_dir == -1:
            return 1
        elif self.MAP.direction - desired_dir == -3 or self.MAP.direction - desired_dir == 1:
            return 1
        else:  # turn twice to turn around
            return 2

    def forward(self):  # go forward
        self.wait_till_done(self.sim_robot.goForward())
        if self.MAP.direction in [0, 2]:
            self.moves_since_cal[0] += 1
        elif self.MAP.direction in [1, 3]:
            self.moves_since_cal[1] += 1
        sleep(self.SLEEP_TIME)

    def calibrate_x(self):  # calibrate on edge
        if self.DEBUG_MODE: print 'CALIBRATING X'
        if self.MAP.robotLoc[0] == 0:
            self.turn(2)
        elif self.MAP.robotLoc[0] == 6:
            self.turn(0)
        self.wait_till_done(self.sim_robot.goCalibrate())
        sleep(self.SLEEP_TIME)

    def calibrate_y(self):
        if self.DEBUG_MODE: print 'CALIBRATING Y'
        if self.MAP.robotLoc[1] == 0:
            self.turn(1)
        elif self.MAP.robotLoc[1] == 6:
            self.turn(3)
        self.wait_till_done(self.sim_robot.goCalibrate())
        sleep(self.SLEEP_TIME)

    def generate_blank_grid(self):  # generate 7x7 grid for path algorithm
        # initialize all values to -1
        grid = []
        for col in range(0, 7):
            col_temp = []
            for row in range(0, 7):
                col_temp.append(-1)
            grid.append(col_temp)
        return grid

    def visit(self):
        # set curr block to visited
        self.MAP.grid[self.MAP.robotLoc[0]][self.MAP.robotLoc[1]].visited = True
        self.MAP.grid[self.MAP.robotLoc[0]][self.MAP.robotLoc[1]].color = (75, 75, 75)
        visited_key = self.MAP.make_location(self.MAP.robotLoc)
        if visited_key in self.goList:
            self.goList.remove(visited_key)
        with open(os.path.join(__location__, 'gridstates.txt'), 'ab') as gridstates:
            gridstates.write('VISITED {}'.format(str(visited_key)) + '\n')

        robotLoc = self.MAP.robotLoc
        # check adjacent blocks for obstructions
        # dir = 0
        # get obstacle report
        obs_report = self.get_obstacle_report()
        #now check specific directions
        if self.MAP.robotLoc[0] >= 0 and self.MAP.robotLoc[0] < 6:
            self.MAP.grid[robotLoc[0] + 1][robotLoc[1]].obstructed = self.see_obstacle(0,obs_report)
            self.MAP.grid[robotLoc[0] + 1][robotLoc[1]].observed = True
            if self.MAP.grid[robotLoc[0] + 1][robotLoc[1]].obstructed:
                self.MAP.grid[robotLoc[0] + 1][robotLoc[1]].color = (255, 165, 0)
        if self.MAP.robotLoc[0] > 0 and self.MAP.robotLoc[0] <= 6:
            self.MAP.grid[robotLoc[0] - 1][robotLoc[1]].obstructed = self.see_obstacle(2,obs_report)
            self.MAP.grid[robotLoc[0] - 1][robotLoc[1]].observed = True
            if self.MAP.grid[robotLoc[0] - 1][robotLoc[1]].obstructed:
                self.MAP.grid[robotLoc[0] - 1][robotLoc[1]].color = (255, 165, 0)
        if self.MAP.robotLoc[1] >= 0 and self.MAP.robotLoc[1] < 6:
            self.MAP.grid[robotLoc[0]][robotLoc[1] + 1].obstructed = self.see_obstacle(3,obs_report)
            self.MAP.grid[robotLoc[0]][robotLoc[1] + 1].observed = True
            if self.MAP.grid[robotLoc[0]][robotLoc[1] + 1].obstructed:
                self.MAP.grid[robotLoc[0]][robotLoc[1] + 1].color = (255, 165, 0)
        if self.MAP.robotLoc[1] > 0 and self.MAP.robotLoc[1] <= 6:
            self.MAP.grid[robotLoc[0]][robotLoc[1] - 1].obstructed = self.see_obstacle(1,obs_report)
            self.MAP.grid[robotLoc[0]][robotLoc[1] - 1].observed = True
            if self.MAP.grid[robotLoc[0]][robotLoc[1] - 1].obstructed:
                self.MAP.grid[robotLoc[0]][robotLoc[1] - 1].color = (255, 165, 0)
        # take readings of cap + EMF
        if (self.wait_till_done(self.sim_robot.readSensor(2))[0] == 1):
            self.MAP.markOT()
            self.sim_robot.set8x8(Robot.translate_coordinate_to_index(robotLoc[1],
                                                                      robotLoc[0]), "T")
        elif (self.wait_till_done(self.sim_robot.readSensor(3))[0] == 1):
            self.MAP.markDeadend()
            self.sim_robot.set8x8(Robot.translate_coordinate_to_index(robotLoc[1],
                                                                      robotLoc[0]), "D")

    def get_obstacle_report(self):
        return self.wait_till_done(self.sim_robot.readSensor(1))

    def see_obstacle(self, direction, obs_report):
        which_sensor = (((direction - self.MAP.direction) % 4) + 1) % 4  # do we need the middle mod 4?
        sensor_val = int(obs_report[which_sensor])
        print sensor_val
        return sensor_val

    def get_vertical_path(self):
        return ['A6', 'A5', 'A4', 'A3', 'A2', 'A1',
                'B1', 'B2', 'B3', 'B4', 'B5', 'B6', 'B7',
                'C7', 'C6', 'C5', 'C4', 'C3', 'C2', 'C1',
                'D1', 'D2', 'D3', 'D4', 'D5', 'D6', 'D7',
                'E7', 'E6', 'E5', 'E4', 'E3', 'E2', 'E1',
                'F1', 'F2', 'F3', 'F4', 'F5', 'F6', 'F7',
                'G7', 'G6', 'G5', 'G4', 'G3', 'G2', 'G1']

    def get_horizontal_path(self):
        return ['B7','C7','D7','E7','F7','G7',
                'G6','F6','E6','D6','C6','B6','A6',
                'A5','B5','C5','D5','E5','F5','G5',
                'G4','F4','E4','D4','C4','B4','A4',
                'A3','B3','C3','D3','E3','F3','G3',
                'G2','F2','E2','D2','C2','B2','A2',
                'A1','B1','C1','D1','E1','F1','G1']

    def get_spiral_path(self):
        return ['A6', 'A5', 'A4', 'A3', 'A2', 'A1',
                'B1', 'C1', 'D1', 'E1', 'F1', 'G1',
                'G2', 'G3', 'G4', 'G5', 'G6', 'G7',
                'F7', 'E7', 'D7', 'C7', 'B7',
                'B6', 'B5', 'B4', 'B3', 'B2',
                'C2', 'D2', 'E2', 'F2',
                'F3', 'F4', 'F5', 'F6',
                'E6', 'D6', 'C6',
                'C5', 'C4', 'C3',
                'D3', 'E3',
                'E4', 'E5',
                'D5', 'D4']

    def get_perimeter_blocks(self):
        # returns horizontal + vertical calib blocks
        hor = ['A1', 'A2', 'A3', 'A4', 'A5', 'A6', 'A7',
               'G1', 'G2', 'G3', 'G4', 'G5', 'G6', 'G7']
        ver = ['A1', 'B1', 'C1', 'D1', 'E1', 'F1', 'G1',
               'A7', 'B7', 'C7', 'D7', 'E7', 'F7', 'G7']
        hor_blocks = []  # horizontal calib blocks
        for loc in hor:
            hor_blocks.append(self.MAP.get_block(loc))
        ver_blocks = []  # vertical calib blocks
        for loc in ver:
            ver_blocks.append(self.MAP.get_block(loc))
        return (hor_blocks, ver_blocks, hor, ver)  # return tuple of lists

    def doStuff(self):

        #light up yellow READY light on 8x8 (A7)
        self.wait_till_done(self.sim_robot.setReadyLight())
        #wait for Go Button to be pressed
        while not int(self.wait_till_done(self.sim_buttons.getGoButton())):
            sleep(0.25)

        # self.goList = self.get_vertical_path()
        self.goList = self.get_horizontal_path()
        # self.goList = self.get_spiral_path()

        with open(os.path.join(__location__, 'gridstates.txt'), 'wb') as gridstates:
            pass

        calib_color = (255, 255, 255)
        # get all perimeter blocks
        perimeter_blocks_full = self.get_perimeter_blocks()
        self.moves_since_cal = [0, 0]
        self.max_moves = [15, 15]
        done = False

        while not done:
            with open(os.path.join(__location__, 'gridstates.txt'), 'ab') as gridstates:
                gridstates.write(str(self.goList) + '\n')

            if not self.MAP.grid[self.MAP.robotLoc[0]][self.MAP.robotLoc[1]].visited:
                self.visit()
            if len(self.goList) == 0:
                self.goList.append('A7')
                done = True
            # break
            with open(os.path.join(__location__, 'gridstates.txt'), 'ab') as gridstates:
                gridstates.write(str(self.goList) + '\n')

            goal = self.goList.pop(0)
            # get path to the goal block, if NOT obstructed
            if self.MAP.get_block(goal).obstructed:
                print '%s is obstructed! skipping...' % goal
                continue  # skip this block if obstructed

            # check if should calib on side
            if self.moves_since_cal[0] >= self.max_moves[0] or self.moves_since_cal[1] >= self.max_moves[1]:
                if self.moves_since_cal[0] >= self.max_moves[0]:
                    # if next 4 blocks will include wanted perim, dont go yet
                    if len(set(perimeter_blocks_full[2]).intersection(set(self.goList[:4]))) == 0 or goal in \
                            perimeter_blocks_full[0]:
                        calib_path = self.get_path_to_block_multi(perimeter_blocks_full[0])
                        self.moves_since_cal[0] = 0
                        if len(calib_path) == 0:
                            dest_block = self.MAP.grid[self.MAP.robotLoc[0]][self.MAP.robotLoc[1]]
                        else:
                            dest_block = calib_path[0]
                        old_color = dest_block.color
                        dest_block.color = calib_color
                        state = self.perform_path(calib_path)
                        #if StopButton was pressed, stop doing stuff
                        if state == None:
                            done = True
                            continue
                        self.calibrate_x()
                        dest_block.color = old_color
                elif self.moves_since_cal[1] >= self.max_moves[1]:
                    # if next 4 blocks will include wanted perim, dont go yet
                    if len(set(perimeter_blocks_full[3]).intersection(set(self.goList[:4]))) == 0 or goal in \
                            perimeter_blocks_full[1]:
                        calib_path = self.get_path_to_block_multi(perimeter_blocks_full[1])
                        self.moves_since_cal[1] = 0
                        if len(calib_path) == 0:
                            dest_block = self.MAP.grid[self.MAP.robotLoc[0]][self.MAP.robotLoc[1]]
                        else:
                            dest_block = calib_path[0]
                        old_color = dest_block.color
                        dest_block.color = calib_color
                        state = self.perform_path(calib_path)
                        #if StopButton was pressed, stop doing stuff
                        if state == None:
                            done = True
                            continue
                        self.calibrate_y()
                        dest_block.color = old_color

            # if not, do normal movements
            do_pathfinding = True
            while do_pathfinding:
                if self.DEBUG_MODE: print 'finding path to %s' % str(goal)
                path = self.get_path_to_block_multi([self.MAP.get_block(goal)])
                finished_following = self.perform_path(path)
                #if returns NONE, that means go button has been pressed
                if finished_following == None:
                    done = True
                    break
                elif finished_following:
                    do_pathfinding = False
                else:
                    if self.DEBUG_MODE: print 'could not follow'
                # go to A7 if done visiting all blocks
                # if len(self.goList) == 0:
                #    self.goList.append('A7')
        print 'done! %s' % str(self.moves_since_cal)
        print 'filling in any obstructed OT or DE...'
        self.fillMapGaps()
        print 'done filling in gaps! ready to stop now'


    def fillMapGaps(self):
        OT_segments = []
        DE_segments = []
        blocks_checked = []
        obstructed_blocks = []
        for col in range(0,7):
            for row in range(0,7):
                #if type T and has not been checked, start looking for adjacent blocks
                curr_block = self.MAP.grid[col][row]
                if curr_block.type == 'T' and curr_block not in blocks_checked:
                    segment = self.createSegment(curr_block,blocks_checked,'T')
                    OT_segments.append(segment)
                elif curr_block.type == 'D' and curr_block not in blocks_checked:
                    segment = self.createSegment(curr_block,blocks_checked,'D')
                    DE_segments.append(segment)
                #also check if obstructed
                if curr_block.obstructed:
                    obstructed_blocks.append(curr_block)
        #now, convert each segment to ([segment],[indexes_of,endpoints])
        New_OT_segments = []
        for segment in OT_segments:
            endpoint_list = []
            relevant_type = 'T'
            for index in range(0,len(segment)):
                block = segment[index] #get block
                relevant_count = 0 #initiate adj T count
                adj_blocks = self.MAP.get_adjacent_blocks(block) #get adj blocks
                for adj_block in adj_blocks:
                    if adj_block.type == 'T':
                        relevant_count += 1
                if relevant_count < 2: #add index to endpoint list if is an endpoint (less than two blocks around)
                    endpoint_list.append(index) 
            New_OT_segments.append((segment,endpoint_list)) #replace segment with tuple (segment,endpoint_list)

        #replace old lists with new lists
        OT_segments = New_OT_segments

        #first, fill in OT
        print 'OT segments: %s' % len(OT_segments)
        print 'DE segments: %s' % len(DE_segments)
        if len(OT_segments) > 1 and len(obstructed_blocks) > 0:
            for obs in obstructed_blocks:
                common_segments = 0
                common_endpoints = 0
                for segment,endpoints in OT_segments:
                    for index in range(0,len(segment)): #look through each block by index
                        block = segment[index]
                        adj_blocks = self.MAP.get_adjacent_blocks(block)
                        found = False
                        for newblock in adj_blocks:
                            if newblock == obs:
                                found = True
                                break 
                        if found:
                            if index in endpoints:
                                common_endpoints += 1
                            else:
                                common_segments += 1
                if common_endpoints == 2 and ((common_endpoints+common_segments) < 3):
                    obs.setOT()
                    obs_loc = self.MAP.get_location(obs.loc)
                    self.sim_robot.set8x8(Robot.translate_coordinate_to_index(obs_loc[1], obs_loc[0]), "T")


    def createSegment(self,curr_block,blocks_checked,type_req):
        #returns (segment,blocks_checked)
        segment = []
        #start segment off with current block
        segment.append(curr_block)
        blocks_checked.append(curr_block)
        #evaluate adjacent blocks
        adj_blocks = self.MAP.get_adjacent_blocks(curr_block)
        for block in adj_blocks:
            if block.type == type_req and block not in blocks_checked:
                #perform this function recursively for block just added
                segment.extend(self.createSegment(block,blocks_checked,type_req))
        return segment


    def perform_path(self, path):
        if int(self.wait_till_done(self.sim_buttons.getStopButton())):
            return None
        if path == None:
            return False
        while len(path) > 0:
            if not self.MAP.grid[self.MAP.robotLoc[0]][self.MAP.robotLoc[1]].visited:
                self.visit()
            desired_block = path.pop(len(path) - 1)
            if desired_block.obstructed:
                return False
            desired_dir = self.calculate_direction(self.MAP.robotLoc, self.MAP.get_location(desired_block.loc))
            if self.DEBUG_MODE: print desired_dir
            self.turn(desired_dir)
            self.forward()
        return True

    def get_path_to_block_multi(self, desired_blocks):  # block from robot MAP
        grid = self.generate_blank_grid()  # get blank grid
        start_block = self.MAP.get_block(self.MAP.make_location(self.MAP.robotLoc))  # starting location
        # check if robot is already in desired blocks
        if start_block in desired_blocks:
            return []  # return empty list
        # generate termination locations
        term_locs = []
        for block in desired_blocks:
            term_locs.append(self.MAP.get_location(block.loc))  # termination blocks
        curr_loc = self.MAP.robotLoc
        block_list = [start_block]

        val = 0
        found = False
        possible = True
        term_block = None
        while not found:
            got_adjacent = False
            new_block_list = []
            for block in block_list:
                # set blocks in list to current value
                curr_loc = self.MAP.get_location(block.loc)
                if self.DEBUG_MODE: print 'curr loc: %s,%s' % (curr_loc[0], curr_loc[1])
                if grid[curr_loc[0]][curr_loc[1]] == -1:
                    grid[curr_loc[0]][curr_loc[1]] = val
                for term_loc in term_locs:
                    if curr_loc[0] == term_loc[0] and curr_loc[1] == term_loc[1]:
                        found = True
                        term_block = self.MAP.grid[term_loc[0]][term_loc[1]]
                        break
                if found == True:
                    break
                # get adjacent blocks from map
                adj_blocks = self.MAP.get_adjacent_blocks(block)
                if self.DEBUG_MODE: print '%s adj blocks: %s' % (val, len(adj_blocks))
                new_list = []
                unvisited_list = []
                # filter out obstructed or unvisited blocks
                for block in adj_blocks:
                    if not block.obstructed:
                        if block.visited or block.observed:
                            block_loc = self.MAP.get_location(block.loc)
                            if grid[block_loc[0]][block_loc[1]] == -1:
                                new_list.append(block)
                        # save into list of unvisited blocks if desired block is unvisited
                        if len(desired_blocks) == 1 and desired_blocks[0].visited != True:
                            block_loc = self.MAP.get_location(block.loc)
                            if grid[block_loc[0]][block_loc[1]] == -1:
                                new_list.append(block)
                            # unvisited_list.append(block)
                if self.DEBUG_MODE: print '%s new blocks: %s' % (val, len(new_list))
                # if there were any adjacent blocks, set to true
                if len(new_list) > 0:
                    got_adjacent = True
                # otherwise, try to use unvisited blocks
                # elif len(unvisited_list) > 0:
                #    got_adjacent = True
                #    new_list.extend(unvisited_list)
                # update block_list
                new_block_list.extend(new_list)
            # update val for next iteration
            if found:
                break
            val += 1
            block_list = new_block_list
            if not got_adjacent:
                possible = False
                break
            if self.DEBUG_MODE: print str(grid)
        if self.DEBUG_MODE: print str(grid)

        # return None if impossible to get to
        if not possible:
            if self.DEBUG_MODE: print 'not possible'
            return None
        # otherwise, build path
        path = [term_block]
        curr_loc = term_loc
        best_dir = None
        while val != 1:
            if self.DEBUG_MODE: print "curr val: %s" % val
            loc_list = self.get_adjacent_grid_blocks(grid, curr_loc)
            new_loc_list = []
            for loc in loc_list:
                if self.DEBUG_MODE: print str(grid)
                if self.DEBUG_MODE: print "x:%s y:%s val:%s" % (loc[0], loc[1], grid[loc[0]][loc[1]])
                if grid[loc[0]][loc[1]] == val - 1:
                    new_loc_list.append(loc)
            # with new loc list, find best loc
            new_curr_loc = None
            if len(new_loc_list) > 1:
                if best_dir == None:
                    # new_curr_loc = new_loc_list[0]
                    # choose path that requires least turns
                    min_turns = 3
                    new_curr_loc = None
                    for new_loc in new_loc_list:
                        temp_turns = self.get_turn_amount(self.calculate_direction(curr_loc, new_loc))
                        if self.DEBUG_MODE: print temp_turns
                        if temp_turns < min_turns:
                            min_turns = temp_turns
                            new_curr_loc = new_loc
                    if self.DEBUG_MODE: print "MIN TURNS: %s" % min_turns
                # new_curr_loc = new_loc_list[0]
                else:
                    for new_loc in new_loc_list:
                        if self.calculate_direction(curr_loc, new_loc) == best_dir:
                            if self.DEBUG_MODE: print 'MATCHES DIRECTION'
                            new_curr_loc = new_loc
                            break
                    if new_curr_loc == None:
                        # choose path that requires least turns
                        min_turns = 3
                        new_curr_loc = None
                        for new_loc in new_loc_list:
                            temp_turns = self.get_turn_amount(self.calculate_direction(curr_loc, new_loc))
                            if self.DEBUG_MODE: print temp_turns
                            if temp_turns < min_turns:
                                min_turns = temp_turns
                                new_curr_loc = new_loc
                        if self.DEBUG_MODE: print "MIN TURNS: %s" % min_turns
                    # new_curr_loc = new_loc_list[0]
            else:
                new_curr_loc = new_loc_list[0]
            best_dir = self.calculate_direction(curr_loc, new_curr_loc)
            curr_loc = new_curr_loc
            path.append(self.MAP.grid[curr_loc[0]][curr_loc[1]])
            val -= 1
        return path  # returns path (list of blocks in reverse order)

    def get_adjacent_grid_blocks(self, grid, loc):  # provide loc as [col,row]
        block_list = []
        if self.DEBUG_MODE: print 'checking loc: %s,%s' % (loc[0], loc[1])
        if loc[0] >= 0 and loc[0] < 6:
            block_list.append([loc[0] + 1, loc[1]])
        if loc[0] > 0 and loc[0] <= 6:
            block_list.append([loc[0] - 1, loc[1]])
        if loc[1] >= 0 and loc[1] < 6:
            block_list.append([loc[0], loc[1] + 1])
        if loc[1] > 0 and loc[1] <= 6:
            block_list.append([loc[0], loc[1] - 1])
        return block_list

    def calculate_direction(self, point1, point2):  # get dirfrom 1 to 2
        if point1[0] == point2[0]:
            if point1[1] > point2[1]:
                return 1
            elif point1[1] < point2[1]:
                return 3
        if point1[1] == point2[1]:
            if point1[0] > point2[0]:
                return 2
            elif point1[0] < point2[0]:
                return 0
