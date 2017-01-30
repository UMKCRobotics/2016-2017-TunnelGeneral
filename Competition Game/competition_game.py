import pygame
import sys, os, getopt
import random
import math
import threading

__location__ = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))  # directory from which this script is ran
sys.path.insert(0, os.path.join(__location__, 'src/'))

from Stage_Competition import Stage_Competition
from Stage_GridChoose import Stage_GridChoose
from Stage_AlgorithmChoose import Stage_AlgorithmChoose
from Stage_Build import Stage_Build
from Sensors import Sensor_Converter
import AI_17
import AI_JED

robotNameToLoad = None


def main(argv):
    global robotNameToLoad
    try:
        opts, args = getopt.getopt(argv, "r:", ["robot="])
    except getopt.GetoptError:
        print 'arguments are -r <robotname>'
    for opt, arg in opts:
        if opt in ("-r", "--robot"):
            robotNameToLoad = arg
            print 'robot %s loaded' % str(arg)


if __name__ == "__main__":
    if not os.path.exists(os.path.join(__location__, 'src/robots')):
        os.mkdir(os.path.join(__location__, 'src/robots'))
    main(sys.argv[1:])

if robotNameToLoad is None:
    robotNameToLoad = 'robot3'

# window position
# so that it isn't placed off-screen on small screens
x = 50
y = 5
os.environ['SDL_VIDEO_WINDOW_POS'] = "%d,%d" % (x, y)

# pygame.mixer.pre_init()
pygame.init()
pygame.mixer.init()
global_grid_width = 100
screen = pygame.display.set_mode((1220, 800))
pygame.display.set_caption('IEEE 2017 UMKC Robotics')
clock = pygame.time.Clock()

build = Stage_Build(screen, saved=robotNameToLoad)
grid_choose = Stage_GridChoose(screen)
alg_choose = Stage_AlgorithmChoose(screen)
comp = Stage_Competition(screen)
stages = [build, grid_choose, alg_choose, comp]

currentStage = 0
previousStage = 0
inputData = None

threads = []


def handle_returnVal(val):
    global currentStage, inputData
    if val[0] == 'QUIT':
        quit_program()
    elif val[0] == 'NEXT_STAGE':
        currentStage += 1
        inputData = val[1]
    elif val[0] == 'PREVIOUS_STAGE':
        currentStage -= 1
        inputData = None


def quit_program():
    pygame.quit()
    sys.exit()


while 1:
    # do stage actions
    if currentStage >= len(stages):
        quit_program()
    if previousStage != currentStage:
        stages[currentStage].stage_input = inputData
        if isinstance(stages[currentStage], Stage_Competition):
            # load board
            if isinstance(inputData[1], list):
                stages[currentStage].board_template = inputData[1]
            else:  # should be int for round number
                stages[currentStage].board_template = stages[currentStage].gameboard.generate_board_round(inputData[1])
            stages[currentStage].gameboard.load_board(stages[currentStage].board_template)

            sensorConv = Sensor_Converter(stages[currentStage].robot, inputData[0][0], inputData[0][1])
            sensorConv.create_robot_sensors()

            #if you want to do 1 step at a time with 'b', set to false
            stages[currentStage].shouldPerformRobotMove = True
            #choose an algorithm to load: AI_17,AI_JED
            algorithm_chosen = inputData[2]
            #create thread for algorithm
            t1 = threading.Thread(target=algorithm_chosen.simulation_impl,args=([stages[currentStage].robot,stages[currentStage].options],))
            t1.daemon = True
            t1.start()
            threads.append(t1)
        previousStage = currentStage
    returnVal = stages[currentStage].performAllStageActions()
    handle_returnVal(returnVal)
    # update screen
    pygame.display.update()
    # set ticks
    clock.tick(60)
