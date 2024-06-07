#!/usr/bin/env python3

import sys
import serial
import time
from tqdm import tqdm
from crc import Calculator, Crc32

if len(sys.argv) != 2:
    print(f'Usage: python {sys.argv[0]} FILE')
    sys.exit(1)

filename = sys.argv[1]
with open(filename, 'rb') as file:
        data = file.read()

filesize=len(data)

calculator = Calculator(Crc32.CRC32)

ser = serial.Serial(
    port='/dev/ttyACM0',
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS
)

ser.isOpen()
ser.write(b'START')

c = b'N'
# Ugly way to detect substring 'START' in response
while True:
    c = ser.read(1)
    if (c == b'S'):
        c = ser.read(1)
        if (c == b'T'):
            c = ser.read(1)
            if (c == b'A'):
                c = ser.read(1)
                if (c == b'R'):
                    c = ser.read(1)
                    if (c == b'T'):
                        break;

blocksize = int(ser.read(4))

print(f'Transaction started, block size is {blocksize} bytes')
ser.write(str(filesize).zfill(8).encode('utf-8'))

c = b'N'
while (c != b'A'):
    c = ser.read(1)

print(f'Sending file \'{filename}\' ({filesize} bytes)')



def required_blocks(filesize, blocksize):
    return (filesize + blocksize - 1) // blocksize

# Pad data with zeroes
data = data + b'0' * (required_blocks(filesize, blocksize) * blocksize - len(data))

retry = 0
naks=0
start_time = time.time()
for block in tqdm(range(required_blocks(filesize, blocksize))):
    chunk = data[block * blocksize : (block + 1) * blocksize]
    checksum = format(calculator.checksum(chunk), 'x').zfill(8).encode('utf-8')
    ser.write(checksum)
    ser.write(chunk)
    c = ser.read(1)
    if (c == b'N'):
        while (retry < 5):
            retry = retry + 1
            naks = naks + 1
            print(f'Received NAK, retry {retry}')
            chunk = data[block * blocksize : (block + 1) * blocksize]
            ser.write(format(calculator.checksum(chunk), 'x').encode('utf-8'))
            ser.write(chunk)
            c = ser.read(1)
            if (c == b'A'):
                break
        if (retry >= 5):
            print(f'Retries exceeded, giving up.')
            sys.exit(1)
        continue
    elif (c != b'A'):
        print(f'Unexpected character: {c}')
        sys.exit(1)
    retry = 0

total_time = time.time() - start_time
speed = filesize / total_time / 1024.0
print(f'Transaction completed successfully ({speed:.2f} kiB/s, {naks} retransmissions)')

while True:
    print(ser.read().decode('utf-8'), end='')

ser.close()
