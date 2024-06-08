#!/usr/bin/env python3

import sys
import serial
import time
import datetime
import tensorflow as tf
import numpy as np
import os
from tqdm import tqdm
from crc import Calculator, Crc32


def get_spectrogram(waveform):
    # Convert the waveform to a spectrogram via a STFT.
    spectrogram = tf.signal.stft(waveform, frame_length=255, frame_step=128)
    # Obtain the magnitude of the STFT.
    spectrogram = tf.abs(spectrogram)
    # Add a `channels` dimension, so that the spectrogram can be used
    # as image-like input data with convolution layers (which expect
    # shape (`batch_size`, `height`, `width`, `channels`).
    spectrogram = spectrogram[..., tf.newaxis]
    return spectrogram

yes = 0
no = 0
# TODO Change the directories to evaluate HERE
for file in tqdm(os.listdir('../ml/data/mini_speech_commands/test/yes')):
    #x = '../ml/data/mini_speech_commands/yes/5184ed3e_nohash_0.wav'
    x = tf.io.read_file(str(os.path.join('../ml/data/mini_speech_commands/test/yes/', file)))
    x, sample_rate = tf.audio.decode_wav(x, desired_channels=1, desired_samples=16000,)
    x = tf.squeeze(x, axis=-1)
    waveform = x
    x = get_spectrogram(x)

    input_data = np.array(x)
    preprocessed_input_data = (input_data * 256).astype('uint8')

    with open('/tmp/input.bin', 'wb') as f:
        f.write(preprocessed_input_data)

    with open('/tmp/input.bin', 'rb') as f:
        data = f.read()

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

    #print(f'Transaction started, block size is {blocksize} bytes')
    ser.write(str(filesize).zfill(8).encode('utf-8'))

    c = b'N'
    while (c != b'A'):
        c = ser.read(1)

    def required_blocks(filesize, blocksize):
        return (filesize + blocksize - 1) // blocksize

    # Pad data with zeroes
    data = data + b'0' * (required_blocks(filesize, blocksize) * blocksize - len(data))

    retry = 0
    naks=0
    start_time = time.time()
    for block in tqdm(range(required_blocks(filesize, blocksize)), leave=False):
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
    #print(f'Transaction completed successfully ({speed:.2f} kiB/s, {naks} retransmissions)')

    # The timeout should be larger than the time it takes for the inference
    # on optimized and unoptimized tensorflow kernels
    ser.timeout = 0.35
    last = False
    dt = 0
    prediction = '?'
    while True:
        c = ser.read().decode('utf-8')
        if (c == ''): #timeout
            break;

        if (c == '#'): # time info
            dt = ser.read(8).decode('utf-8')
            dt = int(dt)

        if last:
            if (c == 'Y'):
                yes = yes + 1
                prediction = 'Y'
            elif (c == 'N'):
                no = no + 1
                prediction = 'N'
        if (c == '@'):
            last = True

    with open('log.csv', 'a') as f:
        # TODO change the percentage calculation HERE
        f.write(f'{datetime.datetime.now().isoformat()},{file},{dt},{prediction},{yes},{no},{yes/(yes+no):.02%}\n')

ser.close()
