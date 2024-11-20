import RPi.GPIO as GPIO

from mpu6050 import mpu6050
import time


def main():

    GPIO.setmode(GPIO.BCM)
    GPIO.setup(23, GPIO.IN)

    imu = mpu6050(0x68)
    # set low pass filter
    imu.set_filter_range(mpu6050.FILTER_BW_10)
    imu.set_accel_range(mpu6050.ACCEL_RANGE_2G)
    imu.set_gyro_range(mpu6050.GYRO_RANGE_250DEG)
    file_time = int(time.time())

    while True:

        if GPIO.input(23):
            file_time = int(time.time())
            print(f"Switch flipped, new file: data{file_time}.csv")
            while GPIO.input(23):
                time.sleep(0.1)
            print("Ready to record data")

        accel_data = imu.get_accel_data()
        gyro_data = imu.get_gyro_data()

        # save data to file
        with open(f"data{file_time}.csv", "a") as f:
            f.write(
                f"{time.time()},{gyro_data['x']},{gyro_data['y']},{gyro_data['z']},{accel_data['x']},{accel_data['y']},{accel_data['z']}\n"
            )


if __name__ == "__main__":
    main()
