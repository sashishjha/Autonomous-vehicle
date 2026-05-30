import serial
import time
ser = serial.Serial('/dev/cu.usbmodem11102', 115200, timeout=1)
ser.dtr = True
ser.rts = True
print("Listening...")
while True:
    line = ser.readline().decode('utf-8', 'ignore').strip()
    if line:
        print("RX:", line)
