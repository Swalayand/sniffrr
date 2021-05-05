import RPi.GPIO as GPIO
import time

butPin = 17 

GPIO.setmode(GPIO.BCM)
GPIO.setup(butPin, GPIO.IN)

try:
    while 1:
        if GPIO.input(butPin):
            print("1", end="", flush=True)
        else:
            print("0", end="", flush=True)
        time.sleep(0.1)
except KeyboardInterrupt:
    GPIO.cleanup()
            
