#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np

data = []
with open('sound.txt', 'r') as f:
    for line in f:
        data.append(int(line))

plt.plot(data, color='black', linewidth=0.5)
plt.show()
