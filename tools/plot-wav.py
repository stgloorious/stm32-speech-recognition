#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np

with open('output.bin', 'rb') as f:
    data = np.frombuffer(f.read(), dtype=np.uint8)

plt.plot(data, color='black', linewidth=0.5)
plt.show()
