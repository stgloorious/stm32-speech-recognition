#!/usr/bin/env python3

import numpy as np
import scipy.io.wavfile as wavfile

def wav_to_uint8(input_wav_path, output_bin_path):
    # Read the WAV file
    sample_rate, data = wavfile.read(input_wav_path)

    # Check if the audio is stereo or mono
    if len(data.shape) == 2:
        # If stereo, convert to mono by averaging the channels
        data = data.mean(axis=1)

    # Normalize the data to be between 0 and 1
    data = data.astype(np.int64)
    data = (data - np.min(data)) / (np.max(data) - np.min(data))

    # Scale to uint8
    data_uint8 = (data * 255).astype(np.uint8)

    # Save the data to a binary file
    data_uint8.tofile(output_bin_path)

    print(f"Successfully converted {input_wav_path} and saved to {output_bin_path}")

# Example usage
input_wav_path = 'input.wav'
output_bin_path = 'output.bin'
wav_to_uint8(input_wav_path, output_bin_path)
