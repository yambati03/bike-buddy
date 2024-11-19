from mpu6050 import mpu6050


def main():
    imu = mpu6050(0x68)
    # set low pass filter
    imu.set_low_pass_filter(mpu6050.FILTER_BW_10)
    imu.set_accel_range(mpu6050.ACCEL_RANGE_2G)
    imu.set_gyro_range(mpu6050.GYRO_RANGE_250DEG)

    while True:
        accel_data = imu.get_accel_data()
        gyro_data = imu.get_gyro_data()
        print(f"Accel: {accel_data}, Gyro: {gyro_data}")

        # time.sleep(1)
