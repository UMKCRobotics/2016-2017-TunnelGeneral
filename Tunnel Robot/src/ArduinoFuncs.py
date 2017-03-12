import threading, os, sys, time  # , serial

from SerialComm import SerialComm
# from SerialCommNoThreading import SerialComm
from DeviceComm import CommRequest


class ArduinoFuncs():
    def __init__(self, serial):
        self.ard = SerialComm(serial)
        self.ard.start()
        # remove above line for no threading

    def setReadyLight(self):
        commandObj = CommRequest('R')
        self.ard.requestCommand(commandObj)
        return commandObj

    def set8x8(self, index, gridType):
        # types:
        # T = objective tunnel,
        # D = dead end,
        # E = empty
        commandObj = CommRequest(gridType + '|' + str(index))
        self.ard.requestCommand(commandObj)
        return commandObj

    def set7segment(self, number):
        commandObj = CommRequest('N' + '|' + str(number))
        if number < 1 or number > 6:
            commandObj.response = 'INVALID'
            commandObj.markDone()
        else:
            self.ard.requestCommand(commandObj)
        return commandObj

    def getGoButton(self):
        commandObj = CommRequest('B' + '|G')
        self.ard.requestCommand(commandObj)
        return commandObj

    def getStopButton(self):
        commandObj = CommRequest('B' + '|S')
        self.ard.requestCommand(commandObj)
        return commandObj

    def getEMFreading(self):
        commandObj = CommRequest('S' + '|E', returnAsList=True)
        self.ard.requestCommand(commandObj)
        return commandObj

    def getObstacleReport(self):
        commandObj = CommRequest('S' + '|O')
        self.ard.requestCommand(commandObj)
        return commandObj

    def performTap(self):
        commandObj = CommRequest('A')
        self.ard.requestCommand(commandObj)
        return commandObj

    def isConnected(self):
        return self.ard.connected

    def moveForward(self):
        commandObj = CommRequest('f')
        self.ard.requestCommand(commandObj)
        return commandObj

    def moveBackward(self):
        commandObj = CommRequest('b')
        self.ard.requestCommand(commandObj)
        return commandObj

    def moveLeft(self):
        print("in arduinofuncs moveLeft function")
        commandObj = CommRequest('l')
        self.ard.requestCommand(commandObj)
        return commandObj

    def moveRight(self):
        commandObj = CommRequest('r')
        self.ard.requestCommand(commandObj)
        return commandObj

    # TO-DO: add value for which side of robot to use
    def goCalibrate(self):
        commandObj = CommRequest('c')
        self.ard.requestCommand(commandObj)
        return commandObj

    def goCalibrateIR(self, side):
        commandObj = CommRequest('c|' + side)
        self.ard.requestCommand(commandObj)
        return commandObj

    def stopThread(self):
        self.ard.keepRunning = False
