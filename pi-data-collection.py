from mpu6050 import mpu6050
import time


def main():
    imu = mpu6050(0x68)
    # set low pass filter
    imu.set_filter_range(mpu6050.FILTER_BW_10)
    imu.set_accel_range(mpu6050.ACCEL_RANGE_2G)
    imu.set_gyro_range(mpu6050.GYRO_RANGE_250DEG)
    file_time = int(time.time())
    print(file_time)
    while True:
        accel_data = imu.get_accel_data()
        gyro_data = imu.get_gyro_data()

        # save data to file
        with open(f"data{file_time}.csv", "a") as f:
            f.write(
                f"{time.time(), gyro_data['x'], gyro_data['y'], gyro_data['z'], accel_data['x'], accel_data['y'], accel_data['z']}\n"
            )


if __name__ == "__main__":
    main()
