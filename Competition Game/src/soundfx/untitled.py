import pygame,time,os


__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__)))

pygame.init()
pygame.mixer.init()
#pygame.mixer.init(frequency=22050, size=-16, channels=2, buffer=4096)
sound = pygame.mixer.Sound(os.path.join(__location__,'smb_jump-small.wav'))
sound.play()
#time.sleep(sound.get_length())
#pygame.mixer.music.load(os.path.join(__location__,"smb_jump-small.ogg"))
#pygame.mixer.music.play()
time.sleep(sound.get_length())
#pygame.mixer.Sound.play()