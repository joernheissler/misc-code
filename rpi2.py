#!/usr/bin/env python3

from RPi import GPIO as gpio
from time import sleep

gpio.setwarnings(False)
gpio.setmode(gpio.BOARD)
gpio.cleanup()

pin = 23
gpio.setup(pin, gpio.OUT)

state = False

try:
    while True:
        state = not state
        gpio.output(pin, state)
        sleep(5)
except:
    pass

gpio.cleanup()
