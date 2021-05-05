import RPi.GPIO as GPIO
import time
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(18,GPIO.OUT)

while True:
    print "."
    GPIO.output(18,GPIO.HIGH)
    time.sleep(0.398125) # A
    GPIO.output(18,GPIO.LOW)
    time.sleep(0.000128) # B
    GPIO.output(18,GPIO.HIGH)
    time.sleep(0.000000750) # C
    GPIO.output(18,GPIO.LOW)
    time.sleep(0.000128) # D
    GPIO.output(18,GPIO.HIGH)
    time.sleep(0.000000750) # E
    GPIO.output(18,GPIO.LOW)
    time.sleep(0.0025) # F 2.5 ms
    GPIO.output(18,GPIO.HIGH)
    time.sleep(0.00000225) # G 2.25 us
    GPIO.output(18,GPIO.LOW)
    time.sleep(0.000161375) # H 2.5 ms
