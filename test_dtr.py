import serial
import time
import os

PORT = os.getenv("ROBOT_PORT", "/dev/cu.usbmodem11102")
BAUD = 115200

print(f"Opening {PORT} with DTR/RTS enabled...")
ser = serial.Serial(PORT, BAUD, timeout=1.0)
ser.dtr = True
ser.rts = True

time.sleep(1)
ser.write(b"HELLO\n")

for _ in range(5):
    if ser.in_waiting:
        print("RX:", ser.read(ser.in_waiting).decode(errors='ignore'))
    time.sleep(0.5)

ser.close()
print("Done")
