import serial
import time


def read_serial_data(port, baudrate, time):
    ser = serial.Serial(port, baudrate)
    while True:
        line = ser.readline().decode("utf-8").strip()  # Read a line from the serial
        with open(f"data{time}.csv", "a") as f:
            f.write(line + "\n")


# # Example usage
if __name__ == "__main__":
    try:
        print(int(time.time()))
        read_serial_data(
            "/dev/cu.usbmodem11301", 57600, int(time.time())
        )  # Adjust the port and baudrate as necessary
    except KeyboardInterrupt:
        print("Exiting...")
