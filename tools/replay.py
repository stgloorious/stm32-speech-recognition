#!/usr/bin/env python3

from plot import plot_data

import simpleaudio as sa
import numpy as np
import wave
import math

sampling_rate = 16000
resolution = 255

def read_data(file_path):
    data = []
    with open(file_path, 'r') as file:
        for line_number, line in enumerate(file, start=1):
            if line_number >= 5:
                try:
                    data.append(float(line.strip()))
                except ValueError:
                    print(f"Error parsing line {line_number}: {line}")
    # Strip first ~100 ms
    start_time = 0.100
    data = data[int(start_time/(1/sampling_rate)):]
    return data

def record_file(file_path, data, sampling_rate):
    with wave.open(file_path, mode="wb") as wav_file:
        wav_file.setnchannels(1)
        wav_file.setsampwidth(1)
        wav_file.setframerate(sampling_rate)
        wav_file.writeframes(bytes(data))

def play_file(file_path):
    wave_obj = sa.WaveObject.from_wave_file(file_path)
    play_obj = wave_obj.play()
    play_obj.wait_done()

if __name__ == "__main__":
    file_path = "data.log"
    data = read_data(file_path)

    # Known-good signal
    #data = []
    #for i in np.arange(6400):
    #    time = 1/sampling_rate * i
    #    amplitude = math.sin(2 * math.pi * 440 * time)
    #    data.append(amplitude)

    # Normalize data
    min_val = np.min(data)
    max_val = np.max(data)
    data = (data - min_val) / (max_val - min_val)
    data = [ int(i) for i in (data * resolution) ]

    plot_data(data)
    record_file("output.wav", data, sampling_rate)
    play_file("output.wav")
