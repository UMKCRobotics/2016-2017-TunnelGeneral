#get serial library for python-2.7.x
import os,sys,time
import serial

#get robot + robotmap for general movement + navigation
__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__))) #directory from which this script is ran
sys.path.insert(0, os.path.join(__location__,'src/'))



print __location__

#insert location from Competition Game
main_dir = os.path.realpath(os.path.join(__location__,'..'))
sys.path.insert(0, os.path.realpath(os.path.join(main_dir,'Competition Game/src')))

print main_dir

from Robot_Impl import Robot_Impl
from Robot import RobotMap
from AI_JED import RobotAlg


robomap = RobotMap(None,0,0,0,shouldPlaySound=False)
robot = Robot_Impl(None,None,robomap)
roboalg = RobotAlg(robot,None)