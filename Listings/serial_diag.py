#!/usr/bin/env python3
"""Just open port and listen for 15 seconds - no DTR toggle."""

import glob, time, serial

port = glob.glob("/dev/cu.usbmodem*")[0]
print(f"Port: {port}")

ser = serial.Serial(port, 115200, timeout=0.1)
# Don't touch DTR/RTS
time.sleep(0.5)
ser.reset_input_buffer()

print("Listening for 15s (no reset)...")
start = time.time()
while time.time() - start < 15:
    if ser.in_waiting:
        raw = ser.read(ser.in_waiting)
        t = time.time() - start
        print(f"  [{t:.1f}s] {raw}")
    time.sleep(0.05)

# Now try sending R byte by byte
print("\nSending R (slow, 5ms gap)...")
for b in b"R\n":
    ser.write(bytes([b]))
    ser.flush()
    time.sleep(0.005)

print("Listening 5s more...")
start = time.time()
while time.time() - start < 5:
    if ser.in_waiting:
        raw = ser.read(ser.in_waiting)
        t = time.time() - start
        print(f"  [{t:.1f}s] {raw}")
    time.sleep(0.05)

ser.close()
print("Done.")
