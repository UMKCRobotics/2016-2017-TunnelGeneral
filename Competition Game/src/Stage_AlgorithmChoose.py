import os
import pygame
import sys

__location__ = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))  # directory from which this script is ran
sys.path.insert(0, os.path.join(__location__))

from Stage import Stage
# algorithms
import AI_17
import AI_JED

class Stage_AlgorithmChoose(Stage):
    algorithms = [AI_17,AI_JED]
        

    def __init__(self, screen):
        Stage.__init__(self, screen)
        self.key_handlers = {
            pygame.K_UP: self.cursor_up,
            pygame.K_DOWN: self.cursor_down,
            pygame.K_LEFT: self.cursor_left,
            pygame.K_RIGHT: self.cursor_right,
            pygame.K_ESCAPE: self.end,
            pygame.K_KP_ENTER: self.choose_this,
            pygame.K_RETURN: self.choose_this
        }
        self.all_buttons = []
        # fill list with buttons
        x = 20
        y = 20
        # random
        self.all_buttons.append(ListButton(screen, x, y, "AI_17", self.algorithms[0]))
        self.all_buttons[0].cursor_here = True
        self.cursored = 0
        y += ListButton.height
        self.all_buttons.append(ListButton(screen, x, y, "AI_JED", self.algorithms[1]))

    def move_cursor_to_index(self, index):
        if self.cursored is None:
            self.cursored = index
            self.all_buttons[self.cursored].cursor_here = True
        else:
            self.all_buttons[self.cursored].cursor_here = False
            self.cursored = index
            self.all_buttons[self.cursored].cursor_here = True

    def cursor_up(self):
        if self.cursored is None:
            self.move_cursor_to_index(0)
            return self.signal_NO_ACTION()
        # cursor is on one of the buttons
        new_index = self.cursored - 1
        if new_index < 0:
            new_index = len(self.all_buttons) - 1
        self.move_cursor_to_index(new_index)
        return self.signal_NO_ACTION()

    def cursor_down(self):
        if self.cursored is None:
            self.move_cursor_to_index(0)
            return self.signal_NO_ACTION()
        # cursor is on one of the buttons
        new_index = self.cursored + 1
        if new_index >= len(self.all_buttons):
            new_index = 0
        self.move_cursor_to_index(new_index)
        return self.signal_NO_ACTION()

    def cursor_left(self):
        return self.signal_NO_ACTION()

    def cursor_right(self):
        return self.signal_NO_ACTION()

    def end(self):
        return self.signal_QUIT()

    def choose_this(self):
        if self.cursored is None:
            return self.signal_NO_ACTION()
        print "input list: "
        print self.stage_input
        return self.signal_NEXT_STAGE(self.stage_input + [self.all_buttons[self.cursored].command])

    def mouse_move(self, position):
        found_button_with_mouse_cursor = None
        for button_index in range(len(self.all_buttons)):
            if self.all_buttons[button_index].mouse_is_here(position):
                found_button_with_mouse_cursor = button_index
        if found_button_with_mouse_cursor is not None:
            self.move_cursor_to_index(found_button_with_mouse_cursor)
        return self.signal_NO_ACTION()

    def mouse_up(self, position):
        if self.all_buttons[self.cursored].mouse_is_here(position):
            # mouse is on the button with the cursor
            return self.choose_this()
        return self.signal_NO_ACTION()

    def handleEvents(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return self.signal_QUIT()
            if event.type == pygame.KEYDOWN:
                try:
                    return self.key_handlers[event.key]()
                except KeyError:
                    pass
            if event.type == pygame.MOUSEMOTION:
                return self.mouse_move(pygame.mouse.get_pos())
            if event.type == pygame.MOUSEBUTTONUP:
                return self.mouse_up(pygame.mouse.get_pos())
        return self.signal_NO_ACTION()

    def performAllStageActions(self):
        returnVal = self.handleEvents()
        self.screen.fill((220, 220, 220))
        for button in self.all_buttons:
            button.draw()
        return returnVal


class ListButton:
    width = 360
    height = 40
    text_color = pygame.Color(30, 30, 30)
    button_colors = {  # key is whether cursor is on this button
        True: pygame.Color(255, 255, 255),
        False: pygame.Color(200, 200, 200)
    }

    def __init__(self, screen, x, y, text, command):
        self.screen = screen
        self.x = x
        self.y = y
        self.command = command
        self.rect = pygame.Rect(x, y, ListButton.width, ListButton.height)
        self.cursor_here = False

        font = pygame.font.Font(None, 24)
        self.text = font.render(text, 1, ListButton.text_color)
        self.text_pos = (x + 10, y + 10)

    def mouse_is_here(self, point):
        return self.rect.collidepoint(point)

    def draw(self):
        pygame.draw.rect(self.screen, ListButton.button_colors[self.cursor_here], self.rect)
        self.screen.blit(self.text, self.text_pos)
