import RPi.GPIO as gpio
import time

gpio.setmode(gpio.BCM)
gpio.setup(2, gpio.IN, pull_up_down=gpio.PUD_DOWN)
last = None
i = 0
while 1:
    while gpio.input(2):...
    print("falling", i)
    i += 1
    while not gpio.input(2):...
    print("rising")
while 1:
    while 1:
        print(gpio.input(2))
    if gpio.input(2):
        new = time.time()
        if last:
            print((new - last))
        last = new
    while gpio.input(2):...
    
