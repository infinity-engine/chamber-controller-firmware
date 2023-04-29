import serial
import os

# Change COM1 to the appropriate serial port
ser = serial.Serial('COM7', 2000000)

# Wait for the directory name to be sent over serial port.
dirname = ser.readline().decode().rstrip()

# Create the directory.
os.makedirs(dirname)

while True:
    # Read a line of data from the serial port.
    line = ser.readline().decode().rstrip()
    if line == '':
        break

    # Split the line into filename and data.
    parts = line.split(':')
    filename = parts[0]
    data = b''.join(parts[1:])

    # Create the file or directory.
    if os.path.dirname(filename) == '':
        # Create a file if the filename doesn't contain a directory path.
        file = open(os.path.join(dirname, filename), 'wb')
        file.write(data)
        file.close()
    else:
        # Create directories if the filename contains directory paths.
        os.makedirs(os.path.join(dirname, os.path.dirname(filename)))
        file = open(os.path.join(dirname, filename), 'wb')
        file.write(data)
        file.close()

# Close the serial port.
ser.close()
