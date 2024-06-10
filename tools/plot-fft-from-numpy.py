#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

# Read the binary file and convert the data to an array of uint8
with open('output.bin', 'rb') as f:
    x = np.frombuffer(f.read(), dtype=np.uint8)

window_size = 256
window_to_inspect = 10
x = x[window_size * window_to_inspect : window_size * (window_to_inspect + 1)]
x = x - np.mean(x)
x = x * np.hanning(256)

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
plt.title('FFT of Test Signal Calculated Using numpy')
plt.xlabel('Frequency (Hz)')
plt.ylabel('Magnitude')
plt.grid(True)
plt.show()
