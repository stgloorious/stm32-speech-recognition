#!/usr/bin/env python3
import matplotlib.pyplot as plt

def read_data(file_path):
    data = []
    with open(file_path, 'r') as file:
        for line_number, line in enumerate(file, start=1):
            if line_number >= 5:
                try:
                    data.append(float(line.strip()))
                except ValueError:
                    print(f"Error parsing line {line_number}: {line}")
    return data

def plot_data(data):
    x = list(range(1, len(data) + 1))  # Sample numbers from 1 to 1600
    plt.figure(figsize=(12, 6))  # Wide figure
    x = [ i*0.0625 for i in x ]
    plt.plot(x, data, color='black', linewidth=0.5)  # Thin black line
    plt.scatter(x, data, color='red', s=3)  # Little red points at each sample
    #plt.xlabel('Sample Number')
    plt.xlabel('Time [ms]')
    plt.ylabel('Amplitude')
    plt.title('Microphone (16 kS/s)')
    plt.grid(True)
    #plt.savefig('plot.pdf')  # Save plot to PDF
    plt.show()

if __name__ == "__main__":
    file_path = "data.log"
    data = read_data(file_path)
    plot_data(data)
