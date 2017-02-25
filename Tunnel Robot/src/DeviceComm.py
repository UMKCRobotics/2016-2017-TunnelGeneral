import threading, os, sys, serial, time

__location__ = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))  # directory from which this script is ran
sys.path.insert(0, os.path.join(__location__))


class CommRequest(object):
    # this class is the object passed into
    # a Device Comm as a request to an
    # outside device
    def __init__(self, request, returnAsList=False):
        self.request = request  # string
        self.isDone = False  # represents if command is done performing
        self.response = None  # optional string value from arduino
        self.returnAsList = returnAsList

    def checkDone(self):
        return self.isDone

    def markDone(self):
        self.isDone = True

    def setResponse(self, response):
        self.response = response

    def getResponse(self):
        return self.response


class DeviceComm(threading.Thread):
    # class used as template for all other
    # comms with outside devices; runs on
    # own thread

    def __init__(self, comm):
        self.comm = comm
        self.keepRunning = True
        self.deviceName = None
        self.commandLock = threading.Lock()
        threading.Thread.__init__(self)
        self.daemon = True
        self.commandList = []

    def requestCommand(self, commReq):
        print("in DeviceComm requestCommand function")
        self.commandLock.acquire()
        print("acquired lock")
        self.commandList.append(commReq)
        self.commandLock.release()
        print("appended and released")

    def removeCommand(self, commReq):
        self.commandLock.acquire()
        self.commandList.remove(commReq)
        self.commandLock.release()

    def run(self):
        self.keepRunning = True
        while (self.keepRunning):
            if len(self.commandList) > 0:  # check if commands to perform
                print("found command in commandList")
                # do the item in queue
                commandObj = self.commandList[0]
                self.performCommand(commandObj)
                # now remove the command from queue
                self.removeCommand(commandObj)
            # wait a little
            time.sleep(0.01)
        # marked for stopping

    def stopThread(self):
        self.keepRunning = False

    def performCommand(self, commReq):
        print("performing command")
        print(commReq)
        # fill this function with comm parsing
