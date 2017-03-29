#!/bin/sh
# main_launcher.sh
# runs main.py

sudo python "/home/pi/2016-2017-TunnelGeneral/Tunnel Robot/main.py" 2>&1 | tee "/home/pi/robotlogs/$(date +"%FT%H%M%S").log"
