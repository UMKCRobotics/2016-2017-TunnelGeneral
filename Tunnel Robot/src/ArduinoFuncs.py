import threading, os, sys, time  #, serial

from SerialComm import SerialComm
from DeviceComm import CommRequest


class ArduinoFuncs():
    def __init__(self, serial):
        self.ard = SerialComm(serial)
        self.ard.start()

    def set8x8(self, index, gridType):
        # types:
        # T = objective tunnel,
        # D = dead end,
        # E = empty
        commandObj = CommRequest(gridType + '|' + str(index))
        self.ard.requestCommand(commandObj)
        return commandObj

    def set7segment(self, number):
        if number < 1 or number > 6:
            return 'INVALID'

        commandObj = CommRequest('N' + '|' + str(number))
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

    def performTap(self):
        commandObj = CommRequest('A' + '|1')
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
        commandObj = CommRequest('l')
        self.ard.requestCommand(commandObj)
        return commandObj

    def moveRight(self):
        commandObj = CommRequest('r')
        self.ard.requestCommand(commandObj)
        return commandObj

    def goCalibrate(self):
        commandObj = CommRequest('c')
        self.ard.requestCommand(commandObj)
        return commandObj

    def stopThread(self):
        self.ard.keepRunning = False
