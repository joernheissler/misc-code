#!/usr/bin/env python

from RPi import GPIO as gpio
from time import sleep
from random import randint as rand

gpio.setwarnings(False)
gpio.setmode(gpio.BOARD)
gpio.cleanup()

pins = [24,26,23]

for pin in pins:
    gpio.setup(pin, gpio.OUT)

state = [False] * len(pins)

while True:
    n = rand(0, 2)
    state[n] = not state[n]
    gpio.output(pins[n], state[n])
    sleep(0.5)
#for i in range(0, 100):
#    for n in xrange(0, 2 ** len(pins)):
#        for pin in range(0, len(pins)):
#            gpio.output(pins[pin], bool(n & 1 << pin))
#        sleep(0.05)

gpio.cleanup()
