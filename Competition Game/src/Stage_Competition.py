import pygame, sys, os, random, math

__location__ = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))  # directory from which this script is ran
sys.path.insert(0, os.path.join(__location__))

from Stage import Stage
from MouseEvents import MouseEvents
from Robot import Robot


class Stage_Competition(Stage):
    Round1Example = [['E', 'E', 'E', 'E', 'TC', 'E', 'E'],
                     ['E', 'E', 'E', 'E', 'T', 'E', 'E'],
                     ['E', 'E', 'E', 'T', 'T', 'E', 'E'],
                     ['E', 'E', 'E', 'T', 'E', 'E', 'E'],
                     ['E', 'E', 'E', 'T', 'E', 'E', 'E'],
                     ['E', 'E', 'E', 'T', 'E', 'E', 'E'],
                     ['E', 'E', 'E', 'T', 'E', 'E', 'E']]

    Round2Example = [['E', 'E', 'E', 'E', 'E', 'T', 'E'],
                     ['E', 'E', 'D', 'E', 'E', 'T', 'E'],
                     ['E', 'E', 'D', 'E', 'T', 'T', 'E'],
                     ['E', 'E', 'D', 'E', 'T', 'E', 'E'],
                     ['T', 'T', 'T', 'T', 'T', 'E', 'E'],
                     ['E', 'E', 'E', 'E', 'E', 'E', 'E'],
                     ['E', 'E', 'E', 'E', 'E', 'E', 'E']]

    Round3Example = [['E', 'E', 'E', 'E', 'E', 'E', 'E'],
                     ['E', 'E', 'T', 'T', 'T', 'E', 'E'],
                     ['E', 'E', 'T', 'E', 'T', 'E', 'E'],
                     ['E', 'E', 'TO', 'E', 'T', 'T', 'T'],
                     ['E', 'E', 'T', 'E', 'E', 'E', 'E'],
                     ['E', 'D', 'T', 'E', 'EO', 'E', 'E'],
                     ['E', 'E', 'T', 'E', 'E', 'E', 'E']]
    FixExample = [['E', 'E', 'E', 'E', 'E', 'E', 'E'],
                  ['E', 'E', 'EO', 'E', 'E', 'E', 'E'],
                  ['E', 'E', 'E', 'E', 'E', 'E', 'E'],
                  ['E', 'E', 'E', 'TO', 'T', 'TO', 'T'],
                  ['E', 'E', 'TO', 'T', 'E', 'E', 'E'],
                  ['E', 'EO', 'T', 'E', 'EO', 'E', 'E'],
                  ['E', 'E', 'T', 'E', 'E', 'E', 'E']]
    FixExample2 = [['E', 'E', 'E', 'E', 'T', 'E', 'E'],
                   ['E', 'EO', 'E', 'EO', 'T', 'EO', 'E'],
                   ['E', 'E', 'EO', 'E', 'TO', 'E', 'E'],
                   ['E', 'EO', 'E', 'E', 'T', 'E', 'E'],
                   ['E', 'E', 'E', 'T', 'TO', 'E', 'E'],
                   ['T', 'T', 'TO', 'T', 'E', 'EO', 'E'],
                   ['E', 'E', 'E', 'E', 'E', 'E', 'E']]

    # BOARD OFFSET VARIABLES
    global_grid_width = 100
    map_grid_width = 50
    global_offsets = (0, 0)
    map_offsets = (850, 50)

    def __init__(self, screen):
        Stage.__init__(self, screen)
        # set stuff up
        # self.global_grid_width = global_grid_width
        self.gameboard = GameBoard(self.screen, self.global_grid_width, self.global_offsets)
        self.robot = Robot(self.screen, self.gameboard, offsets=self.map_offsets)

        # load in a board
        # self.gameboard.load_board(self.Round3Example)
        self.gameboard.load_board(self.gameboard.generate_board_round(3))
        # self.gameboard.load_board(self.FixExample2)

        # choose visibility options to start
        self.gameboard.visible = True
        self.gameboard.showGrid = True
        # set up options menu
        self.options = Options(self.screen, (850, 450))
        # set up objects to be drawn
        self.global_objects = []
        self.global_objects.append(self.gameboard)
        self.global_objects.append(self.robot.MAP)
        self.global_objects.append(self.options)
        self.mouse = MouseEvents(self.global_objects)

    # self.counter = 1
    # self.delayCount = 144

    def handleEvents(self):
        mouse = self.mouse
        robot = self.robot
        gameboard = self.gameboard
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return self.signal_QUIT()
            if event.type == pygame.KEYDOWN:
                robot.handleKeyEvent(event)
                if event.key in [pygame.K_v]:
                    gameboard.toggleVisible()
                if event.key in [pygame.K_g]:
                    gameboard.toggleGrid()
                if event.key in [pygame.K_ESCAPE]:
                    return self.signal_QUIT()
            if event.type in [pygame.MOUSEBUTTONDOWN, pygame.MOUSEBUTTONUP]:
                mouse.handleMouseEvent(event)
        return self.signal_NO_ACTION()

    def performAllStageActions(self):
        mouse = self.mouse
        returnVal = self.handleEvents()
        mouse.performActions()
        self.screen.fill((220, 220, 220))
        for obj in self.global_objects:
            obj.draw()
        # self.counter += 1
        # if self.counter % self.delayCount == 0:
        self.robot.performMove()
        return returnVal


# GameBoard: actual representation
class GameBoard():
    ROBOT = None

    def __init__(self, screen, grid_width, offsets):
        self.screen = screen
        self.GRID_WIDTH = grid_width
        self.OFFSETS = offsets
        self.TOTAL_WIDTH = self.GRID_WIDTH * 8
        self.TOTAL_HEIGHT = self.TOTAL_WIDTH
        self.BASE = pygame.Rect(self.OFFSETS[0], self.OFFSETS[1], self.TOTAL_WIDTH, self.TOTAL_WIDTH)
        self.PLEXI = pygame.Rect(self.BASE.x + self.GRID_WIDTH / 2, self.BASE.y + self.GRID_WIDTH / 2,
                                 self.GRID_WIDTH * 7, self.GRID_WIDTH * 7)
        self.grid = []
        self.visible = False
        self.showGrid = False
        self.generate_blocks()
        self.obstructions = []
        self.powerlines = []
        self.empty = []

    def generate_board_round(self, roundNumber):
        if roundNumber == 1:
            # at most 2 turns, no obstructions, no deadends
            return self.generate_random_board((0, 2), (0, 0), False)
        elif roundNumber == 2:
            # at most 3 turns, no obstructions, possible deadends
            return self.generate_random_board((0, 3), (0, 0), True)
        elif roundNumber == 3:
            # at most 3 turns, at least 1 obstruction, possible deadends
            return self.generate_random_board((0, 3), (1, 5), True)

    def generate_random_board(self, turnsRange, obstaclesRange, haveDeadends):
        # turnsRange is (minTurns,maxTurns)
        # obstaclesRange is (minObstacles,maxObstacles)
        # haveDeadends is True or False

        # create list of perimeter blocks to choose from
        per_lists = self.get_perimeter_locations()
        COMPLETED = False
        while not COMPLETED:
            template_grid = self.generate_blank_grid('E')

            try:
                # first, choose a starting side + block for tunnel
                starting_side = random.choice(per_lists)
                starting_loc = random.choice(starting_side)
                template_grid[starting_loc[0]][starting_loc[1]] = 'T'

                min_length = 7
                # initialize counters for generation
                FIRST_RUN = True
                old_direction = None
                done = False
                turns = 0
                length = 1
                obstacles = 0
                curr_loc = starting_loc
                print 'STARTING LOC: %s' % str(curr_loc)
                # main loop
                while not done:
                    print 'CURR LOC: %s' % str(curr_loc)
                    print str(template_grid)
                    # get adjacent blocks
                    adjacent_blocks = self.get_adjacent_grid_blocks(template_grid, curr_loc)
                    # if length not at minimum, exclude perimeter blocks
                    if length < min_length or turns < turnsRange[0]:
                        new_adjacent_blocks = []
                        for block in adjacent_blocks:
                            if self.grid[block[0]][block[1]].perimeter == True:
                                continue
                            else:
                                new_adjacent_blocks.append(block)
                    else:  # otherwise, do nothing of note here
                        new_adjacent_blocks = adjacent_blocks
                    # exclude any blocks that are adjacent to tunnel; ignore
                    adjacent_blocks = new_adjacent_blocks
                    new_adjacent_blocks = []
                    for block in adjacent_blocks:
                        temp_adjacent_blocks = self.get_adjacent_grid_blocks(template_grid, block)
                        total_adj_tunnels = 0
                        # check if this location includes a T
                        if 'T' in template_grid[block[0]][block[1]]:
                            continue  # skip this block
                        print 'TEMP BLOCKS FOR %s: %s' % (str(block), str(temp_adjacent_blocks))
                        for temp_block in temp_adjacent_blocks:
                            print 'VAL IN TEMP %s: %s' % (str(temp_block), template_grid[temp_block[0]][temp_block[1]])
                            if 'T' in template_grid[temp_block[0]][temp_block[1]]:
                                total_adj_tunnels += 1
                        if total_adj_tunnels < 2:
                            if turns == turnsRange[1]:
                                if self.calculate_direction(curr_loc, block) != old_direction:
                                    continue
                            new_adjacent_blocks.append(block)

                    print 'FILTERED BLOCKS: %s' % str(new_adjacent_blocks)
                    # now that a finalized list of available blocks is available, choose it
                    new_curr_loc = random.choice(new_adjacent_blocks)
                    # check if perimeter block chosen: if so, done creating path
                    if new_curr_loc[0] in [0, 6] or new_curr_loc[1] in [0, 6]:
                        done = True

                    if FIRST_RUN:  # set old_direction on first run
                        old_direction = self.calculate_direction(curr_loc, new_curr_loc)
                        FIRST_RUN = False
                    else:  # compare to old direction on subsequent runs
                        new_direction = self.calculate_direction(curr_loc, new_curr_loc)
                        if new_direction != old_direction:
                            turns += 1
                        old_direction = new_direction

                    curr_loc = new_curr_loc
                    length += 1
                    template_grid[curr_loc[0]][curr_loc[1]] = 'T'
            except IndexError:
                print 'CANT FINISH TUNNEL, RETRYING...'
            else:
                # check if both endpoints are not on the same side
                valid = True
                for perim in per_lists:
                    tunnel_count = 0
                    for block in perim:
                        if 'T' in template_grid[block[0]][block[1]]:
                            tunnel_count += 1
                    if tunnel_count >= 2:
                        valid = False
                        'ENDPOINTS ON SAME SIDE, INVALID'
                        break
                # if endpoints on diff sides, good to go
                if valid:
                    COMPLETED = True

        # add deadends (if any)
        if haveDeadends:
            deadend_count = 0
            should_add = True
            while should_add:
                if deadend_count < 1:
                    should_add = True
                else:
                    should_add = random.choice([False, True])
                # if should not add, don't add any more
                if not should_add:
                    continue
                # get all relevant OT and DE blocks thus far
                relevant_blocks = []
                for x in range(1, 6):
                    for y in range(1, 6):
                        if 'T' in template_grid[x][y]:
                            relevant_blocks.append([x, y])
                        if 'D' in template_grid[x][y]:
                            relevant_blocks.append([x, y])
                # get list of possible choices
                possible_blocks = []
                for rel_block in relevant_blocks:
                    blocks = self.get_adjacent_grid_blocks(template_grid, rel_block)
                    for block in blocks:
                        if block not in relevant_blocks:
                            if block[0] not in [0, 6] and block[1] not in [0, 6]:
                                possible_blocks.append(block)
                # remove any blocks that have invalid adjacent blocks
                choice_blocks = []
                for pos_block in possible_blocks:
                    adj_tunnel_count = 0
                    blocks = self.get_adjacent_grid_blocks(template_grid, pos_block)
                    for block in blocks:
                        if 'T' in template_grid[block[0]][block[1]] or 'D' in template_grid[block[0]][block[1]]:
                            adj_tunnel_count += 1
                    if adj_tunnel_count < 2:
                        choice_blocks.append(pos_block)
                if len(choice_blocks) > 0:
                    block_to_add = random.choice(choice_blocks)
                    template_grid[block_to_add[0]][block_to_add[1]] = 'D'
                else:
                    should_add = False
                deadend_count += 1

        # add obstructions (if any)
        obs_num = random.randint(obstaclesRange[0], obstaclesRange[1])
        for n in range(0, obs_num):
            found_loc = False
            x_num = None
            y_num = None
            while not found_loc:
                found_loc = True
                x_num = random.randint(1, 5)
                y_num = random.randint(1, 5)
                blocks = self.get_adjacent_grid_blocks(template_grid, [x_num, y_num])
                blocks.append([x_num, y_num])
                for block in blocks:
                    if 'O' in template_grid[block[0]][block[1]]:
                        found_loc = False
                    elif 'D' in template_grid[block[0]][block[1]]:
                        found_loc = False
            template_grid[x_num][y_num] += 'O'

        # return completed template
        with open(os.path.join(__location__, 'genmap.txt'), 'wb') as genmap:
            for col in template_grid:
                genmap.write(str(col) + '\n')
        return template_grid

    def get_adjacent_grid_blocks(self, grid, loc):  # provide loc as [col,row]
        block_list = []
        print 'checking loc: %s,%s' % (loc[0], loc[1])
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

    def generate_blank_grid(self, default_content):  # generate 7x7 grid for path algorithm
        # initialize all values to -1
        grid = []
        for col in range(0, 7):
            col_temp = []
            for row in range(0, 7):
                col_temp.append(default_content)
            grid.append(col_temp)
        return grid

    def get_perimeter_locations(self):
        top = [[1, 0], [2, 0], [3, 0], [4, 0], [5, 0]]
        bottom = [[1, 6], [2, 6], [3, 6], [4, 6], [5, 6]]
        left = [[0, 1], [0, 2], [0, 3], [0, 4], [0, 5]]
        right = [[6, 1], [6, 2], [6, 3], [6, 4], [6, 5]]
        # return in order from dir 0 - 3
        return (right, top, left, bottom)

    def draw(self):
        # draw base board
        self.BASE.topleft = self.OFFSETS
        self.PLEXI.topleft = (self.OFFSETS[0] + self.GRID_WIDTH / 2, self.OFFSETS[1] + self.GRID_WIDTH / 2)
        self.screen.fill((185, 156, 107), self.BASE)
        self.draw_blocks()
        if self.showGrid:
            self.draw_grid()
        try:
            self.ROBOT.draw()
        except Exception, e:
            print str(e)

    def get_block(self, blockName):
        col = 'ABCDEFG'.find(blockName[0])
        row = '1234567'.find(blockName[1])
        return self.grid[col][row]

    def get_block_in_loc(self, blockCoord):
        # print blockCoord
        return self.grid[blockCoord[0]][blockCoord[1]]

    def draw_grid(self):
        linecolor = (255, 255, 255)
        for n in range(0, 8):
            pygame.draw.lines(self.screen, linecolor, False, [
                (self.GRID_WIDTH / 2 + self.GRID_WIDTH * n + self.OFFSETS[0], self.OFFSETS[1] + self.GRID_WIDTH / 2),
                (self.GRID_WIDTH / 2 + self.GRID_WIDTH * n + self.OFFSETS[0],
                 self.TOTAL_WIDTH + self.OFFSETS[1] - self.GRID_WIDTH / 2)], 1)
        for n in range(0, 8):
            pygame.draw.lines(self.screen, linecolor, False, [
                (self.OFFSETS[0] + self.GRID_WIDTH / 2, self.GRID_WIDTH / 2 + self.GRID_WIDTH * n + self.OFFSETS[1]),
                (self.TOTAL_WIDTH + self.OFFSETS[0] - self.GRID_WIDTH / 2,
                 self.GRID_WIDTH / 2 + self.GRID_WIDTH * n + self.OFFSETS[1])], 1)

    def draw_blocks(self):
        for col in self.grid:
            for block in col:
                block.draw()

    def generate_blocks(self):
        rows = '1234567'
        cols = 'ABCDEFG'
        for col in range(0, 7):
            col_list = []
            for row in range(0, 7):
                col_list.append(
                    GridBlock(self.screen, self, (col * self.GRID_WIDTH + self.OFFSETS[0] + self.GRID_WIDTH / 2,
                                                  row * self.GRID_WIDTH + self.OFFSETS[1] + self.GRID_WIDTH / 2),
                              self.GRID_WIDTH, cols[col] + rows[row]))
            self.grid.append(col_list)
        # set perimeter + corner values
        for n in range(0, 7):
            self.grid[n][0].perimeter = True
            self.grid[n][6].perimeter = True
            if n == 0 or n == 6:
                self.grid[n][0].corner = True
                self.grid[n][6].corner = True
        for n in range(1, 6):
            self.grid[0][n].perimeter = True
            self.grid[6][n].perimeter = True

    def load_board(self, template):
        self.powerlines = []
        self.empty = []
        self.obstructions = []
        if len(template) != 7:
            print "could not load in board; not the right num of rows"
            return False
        for col in range(0, 7):
            for row in range(0, 7):
                # print type(self.grid[col][row])
                if 'T' in template[row][col]:
                    self.grid[col][row] = OT_Block(self.grid[col][row])
                    self.grid[col][row].visible = True
                    self.empty.append(self.grid[col][row])
                    self.powerlines.extend(self.grid[col][row].powerlines)
                elif 'D' in template[row][col]:
                    self.grid[col][row] = DeadEnd_Block(self.grid[col][row])
                    self.grid[col][row].visible = True
                    self.empty.append(self.grid[col][row])
                if 'O' in template[row][col]:
                    self.grid[col][row].createObstruction()
                    self.obstructions.append(self.grid[col][row].obstruction)
                if 'C' in template[row][col]:
                    self.grid[col][row].cache = random.randint(1, 6)
        print 'load completed, initializing powerlines...'
        for row in range(0, 7):
            for col in range(0, 7):
                self.grid[col][row].initialize()
        print 'done initializing powerlines'

    def handleMouseEvent(self, event):
        pass

    def toggleVisible(self):
        self.visible = not self.visible

    def toggleGrid(self):
        self.showGrid = not self.showGrid


class Obstruction():
    def __init__(self, gridblock):
        self.color = (255, 99, 71)
        self.gridblock = gridblock
        self.object = pygame.Rect(gridblock.object.topleft, gridblock.object.size)
        self.touched = False

    def draw(self):
        self.object.topleft = self.gridblock.object.topleft
        self.gridblock.screen.fill(self.color, self.object)


# Grid Block: actual block of board (49 in total)
class GridBlock():
    rows = '1234567'
    cols = 'ABCDEFG'

    def __init__(self, screen, gameboard, coords, grid_width, grid_key):
        self.screen = screen
        self.GAMEBOARD = gameboard
        self.coords = coords
        self.GRID_WIDTH = grid_width
        self.loc = grid_key
        self.color = (0, 0, 0)
        self.color_hidden = (0, 0, 0)
        self.object = pygame.Rect(self.coords, (self.GRID_WIDTH, self.GRID_WIDTH))
        self.obstruction = None
        self.type = 'EMPTY'
        self.cache = None
        self.visible = False
        self.perimeter = False
        self.corner = False

    def get_location(self, blockName):
        col = self.cols.find(blockName[0])
        row = self.rows.find(blockName[1])
        return [col, row]

    def make_location(self, coordList):
        col = self.cols[coordList[1]]
        row = self.rows[coordList[0]]
        return col + row

    def createObstruction(self):
        self.obstruction = Obstruction(self)

    def initialize(self):
        pass

    def draw(self):
        self.object.topleft = (self.GAMEBOARD.OFFSETS[0] + self.coords[0], self.GAMEBOARD.OFFSETS[1] + self.coords[1])
        if self.GAMEBOARD.visible and self.visible:
            self.screen.fill(self.color, self.object)
            self.draw_special()
        else:
            self.screen.fill(self.color_hidden, self.object)
        if self.GAMEBOARD.visible and self.obstruction:
            self.obstruction.draw()

    def draw_special(self):
        pass  # change in specific blocks


class OT_Block(GridBlock):
    def __init__(self, sgb):  # Some Grid BLock
        GridBlock.__init__(self, sgb.screen, sgb.GAMEBOARD, sgb.coords, sgb.GRID_WIDTH, sgb.loc)
        self.color = (0, 255, 255)
        self.type = 'OT'
        # self.powerlines = [pygame.Rect(self.object.topleft,(0,0)),pygame.Rect(self.object.topleft,(0,0))]
        self.powerlines = [PowerLine(self, (0, 0)), PowerLine(self, (0, 0))]

    def draw_special(self):
        for power in self.powerlines:
            power.draw()

    def initialize(self):
        currentGB = self.get_location(self.loc)
        powerNum = 0
        # check left
        if powerNum < 2 and (currentGB[0] <= 0 or self.GAMEBOARD.get_block_in_loc(
                (currentGB[0] - 1, currentGB[1])).type == self.type):
            self.powerlines[powerNum].object.size = (self.GRID_WIDTH * 2 / 3, self.GRID_WIDTH / 3)
            self.powerlines[powerNum].object.left = self.object.left
            self.powerlines[powerNum].object.centery = self.object.centery
            self.powerlines[powerNum].coords = (self.powerlines[powerNum].object.topleft[0] - self.object.topleft[0],
                                                self.powerlines[powerNum].object.topleft[1] - self.object.topleft[1])
            powerNum += 1
        # check right
        if powerNum < 2 and (currentGB[0] >= 6 or self.GAMEBOARD.get_block_in_loc(
                (currentGB[0] + 1, currentGB[1])).type == self.type):
            self.powerlines[powerNum].object.size = (self.GRID_WIDTH * 2 / 3, self.GRID_WIDTH / 3)
            self.powerlines[powerNum].object.right = self.object.right
            self.powerlines[powerNum].object.centery = self.object.centery
            self.powerlines[powerNum].coords = (self.powerlines[powerNum].object.topleft[0] - self.object.topleft[0],
                                                self.powerlines[powerNum].object.topleft[1] - self.object.topleft[1])
            powerNum += 1
        # check above
        if powerNum < 2 and (currentGB[1] <= 0 or self.GAMEBOARD.get_block_in_loc(
                (currentGB[0], currentGB[1] - 1)).type == self.type):
            self.powerlines[powerNum].object.size = (self.GRID_WIDTH / 3, self.GRID_WIDTH * 2 / 3)
            self.powerlines[powerNum].object.top = self.object.top
            self.powerlines[powerNum].object.centerx = self.object.centerx
            self.powerlines[powerNum].coords = (self.powerlines[powerNum].object.topleft[0] - self.object.topleft[0],
                                                self.powerlines[powerNum].object.topleft[1] - self.object.topleft[1])
            powerNum += 1
        # check below
        if powerNum < 2 and (currentGB[1] >= 6 or self.GAMEBOARD.get_block_in_loc(
                (currentGB[0], currentGB[1] + 1)).type == self.type):
            self.powerlines[powerNum].object.size = (self.GRID_WIDTH / 3, self.GRID_WIDTH * 2 / 3)
            self.powerlines[powerNum].object.bottom = self.object.bottom
            self.powerlines[powerNum].object.centerx = self.object.centerx
            self.powerlines[powerNum].coords = (self.powerlines[powerNum].object.topleft[0] - self.object.topleft[0],
                                                self.powerlines[powerNum].object.topleft[1] - self.object.topleft[1])
            powerNum += 1
        # print "%s:%s:%s" % (self.loc,powerNum,currentGB)


class PowerLine():
    def __init__(self, gridblock, coords):
        self.gridblock = gridblock
        self.object = pygame.Rect(self.gridblock.object.topleft, (0, 0))
        self.coords = coords
        self.color = (255, 0, 0)

    def draw(self):
        self.object.topleft = (
        self.gridblock.object.topleft[0] + self.coords[0], self.gridblock.object.topleft[1] + self.coords[1])
        self.gridblock.screen.fill(self.color, self.object)


class DeadEnd_Block(GridBlock):
    def __init__(self, sgb):  # Some Grid BLock
        GridBlock.__init__(self, sgb.screen, sgb.GAMEBOARD, sgb.coords, sgb.GRID_WIDTH, sgb.loc)
        self.special_color = (0, 255, 255)
        self.type = 'DEADEND'

    def draw_special(self):
        self.screen.fill(self.special_color, self.object)


class Options():
    def __init__(self, screen, offsets, objects=[]):
        self.screen = screen
        self.OBJECTS = objects
        self.OFFSETS = offsets
        self.TOTAL_WIDTH = 150
        self.TOTAL_HEIGHT = 250
        self.color = (100, 100, 100)
        self.object = pygame.Rect(self.OFFSETS, (self.TOTAL_WIDTH, self.TOTAL_HEIGHT))
        self.createButtons()

    def draw(self):
        self.object.topleft = self.OFFSETS
        self.screen.fill(self.color, self.object)
        for obj in self.OBJECTS:
            obj.draw()

    def createButtons(self):
        # GO BUTTON
        goButton = Button(self, (10, self.TOTAL_HEIGHT - 70), (50, 50))
        goButton.color = (0, 255, 0)
        goButton.text_content = 'GO'
        self.OBJECTS.append(goButton)
        # STOP BUTTON
        stopButton = Button(self, (self.TOTAL_WIDTH - 60, self.TOTAL_HEIGHT - 70), (50, 50))
        stopButton.color = (255, 0, 0)
        stopButton.text_content = 'STOP'
        self.OBJECTS.append(stopButton)

    def handleMouseEvent(self, event):
        pass


class Button():
    coords = None

    def __init__(self, menu, coords, size):
        self.MENU = menu
        self.coords = coords
        self.object = pygame.Rect(self.coords, size)
        self.color = (0, 0, 0)
        self.text_color = (255, 255, 255)
        self.text_content = "Sample"
        self.font = pygame.font.Font(None, 16)

    def draw(self):
        self.text = self.font.render(self.text_content, 1, self.text_color)
        self.object.topleft = (self.MENU.OFFSETS[0] + self.coords[0], self.MENU.OFFSETS[1] + self.coords[1])
        self.MENU.screen.fill(self.color, self.object)
        # draw text
        textpos = self.text.get_rect()
        textpos.center = self.object.center
        self.MENU.screen.blit(self.text, textpos)

    def handleMouseEvent(self, event):
        pass
