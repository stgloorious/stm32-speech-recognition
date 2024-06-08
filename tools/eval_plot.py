#!/usr/bin/env python3

import pandas as pd
import matplotlib.pyplot as plt

# File paths
files = {
    "optimized": ["eval_data/no-125-cmsisnn.csv", "eval_data/yes-125-cmsisnn.csv"],
    "unoptimized": ["eval_data/no-125-unoptimized.csv", "eval_data/yes-125-unoptimized.csv"]
}

# Function to extract the latency value from the third column of the last line
def extract_latency(file_path):
    # Read the CSV file
    df = pd.read_csv(file_path, header=None)
    # Get the last row, third column
    latency = int(df.iloc[-1, 2])
    return latency

# Extracting and averaging latencies
average_latencies = {}
for key, paths in files.items():
    latencies = [extract_latency(path) for path in paths]
    average_latencies[key] = sum(latencies) / len(latencies)

# Prepare data for plotting
categories = ['Optimized', 'Unoptimized']
latencies = [average_latencies['optimized'], average_latencies['unoptimized']]

# Plotting
x = range(len(categories))  # the label locations
width = 0.5  # the width of the bars

fig, ax = plt.subplots()
bars = ax.bar(x, latencies, width, color=['blue', 'red'])

# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_xlabel('Optimization')
ax.set_ylabel('Average Latency (ms)')
ax.set_title('Latency Reduction By Using Optimized TFLite Kernel (n=250)')
ax.set_xticks(x)
ax.set_xticklabels(categories)
ax.grid(True, axis='y')  # Add horizontal grid
ax.set_axisbelow(True)

# Attach a text label above each bar in rects, displaying its height
def autolabel(bars):
    for bar in bars:
        height = bar.get_height()
        ax.annotate('{}'.format(height),
                    xy=(bar.get_x() + bar.get_width() / 2, height),
                    xytext=(0, 3),  # 3 points vertical offset
                    textcoords="offset points",
                    ha='center', va='bottom')

autolabel(bars)

fig.tight_layout()

plt.show()

