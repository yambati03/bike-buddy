import RPi.GPIO as GPIO
import time

PIN = 23

GPIO.setmode(GPIO.BCM)

GPIO.setup(PIN, GPIO.IN)

try:
    while True:
        print(GPIO.input(PIN))
        time.sleep(0.1)  # Add a small delay to avoid overwhelming the CPU
except KeyboardInterrupt:
    print("Exiting program")
finally:
    GPIO.cleanup()  # Clean up GPIO settings
