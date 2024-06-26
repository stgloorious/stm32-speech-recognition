#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

# Read the binary file and convert the data to an array of uint8
X = []
with open('data.txt', 'r') as f:
    for line in f:
        X.append(float(line.strip()))

# Get the first FFT (N+1 points)
X = X[129 * 20 : 129 * 20 + 129]
samplingrate = 16000
N = len(X)*2
n = np.arange(N)
T = N / samplingrate
freq = n / T

# Plot the FFT results with frequency in hertz
plt.figure(figsize=(10, 6))
plt.plot(freq[:N//2], X)  # Plot only the positive frequencies
plt.title('FFT of Test Signal Calculated By STM32')
plt.xlabel('Frequency (Hz)')
plt.ylabel('Magnitude')
plt.grid(True)
plt.show()
