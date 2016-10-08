import pygame, sys


class Stage():
    def __init__(self, screen, stage_input=None):
        self.stage_input = stage_input
        self.global_objects = []
        self.screen = screen

    def handleEvents(self):
        pass

    def performAllStageActions(self):
        return None

    def signal_QUIT(self):
        return ('QUIT', None)

    def signal_NO_ACTION(self):
        return (None, None)

    def signal_NEXT_STAGE(self, data):
        return ('NEXT_STAGE', data)
