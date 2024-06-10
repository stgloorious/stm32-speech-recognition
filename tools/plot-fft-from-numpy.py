#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

# Read the binary file and convert the data to an array of uint8
with open('output.bin', 'rb') as f:
    x = np.frombuffer(f.read(), dtype=np.int8)

x = x[256*10:256*11]
x = x - np.mean(x)
print(f'mean={np.mean(x)}')

samplingrate = 16000

# Compute the FFT of the data
X = np.abs(np.fft.fft(x))
N = len(X)
n = np.arange(N)
T = N / samplingrate
freq = n / T

# Plot the FFT results with frequency in hertz
plt.figure(figsize=(10, 6))
plt.plot(freq[:N//2], X[:N//2])  # Plot only the positive frequencies
plt.title('FFT of 440 Hz Test Signal Calculated Using numpy')
plt.xlabel('Frequency (Hz)')
plt.ylabel('Magnitude')
plt.grid(True)
plt.show()
