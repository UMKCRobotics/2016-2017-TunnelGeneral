import pygame, sys, os, random, math


class MouseEvents():  # handles clicking/dragging boards/objects
    coords = (0, 0)
    rel_coords = (0, 0)
    held = False
    curObject = None

    def __init__(self, objects):
        self.OBJECTS = objects

    def handleMouseEvent(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN:
            self.held = True
            self.coords = pygame.mouse.get_pos()
            for n in range(len(self.OBJECTS) - 1, -1, -1):
                obj = self.OBJECTS[n]
                tempRect = pygame.Rect(obj.OFFSETS[0], obj.OFFSETS[1], obj.TOTAL_WIDTH, obj.TOTAL_HEIGHT)
                if tempRect.collidepoint(self.coords):
                    self.curObject = obj
                    self.rel_coords = (self.coords[0] - obj.OFFSETS[0], self.coords[1] - obj.OFFSETS[1])
                    # switch first item with item found
                    self.OBJECTS[len(self.OBJECTS) - 1], self.OBJECTS[n] = self.OBJECTS[n], self.OBJECTS[
                        len(self.OBJECTS) - 1]
                    # pass event to object
                    self.curObject.handleMouseEvent(event)
                    break  # do not search for further items

        elif event.type == pygame.MOUSEBUTTONUP:
            self.held = False
            self.curObject = None
        # print "mouse up"

    def performActions(self):
        if self.curObject != None and self.held:
            # print "dragging!"
            self.coords = pygame.mouse.get_pos()
            self.curObject.OFFSETS = (self.coords[0] - self.rel_coords[0], self.coords[1] - self.rel_coords[1])
