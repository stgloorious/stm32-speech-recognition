#!/usr/bin/env python3

import tensorflow as tf
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import os
import math

window_size = 256
frame_step = 128
samplingrate = 16000

norm = mcolors.Normalize(-10, 2)

# Apply STFT to waveforms to get spectrogram
def get_spectrogram(waveform):
  # Convert the waveform to a spectrogram via a STFT.
  spectrogram = tf.signal.stft(waveform, frame_length=window_size, frame_step=frame_step)
  # Obtain the magnitude of the STFT.
  spectrogram = tf.abs(spectrogram)
  # Add a `channels` dimension, so that the spectrogram can be used
  # as image-like input data with convolution layers (which expect
  # shape (`batch_size`, `height`, `width`, `channels`).
  spectrogram = spectrogram[..., tf.newaxis]
  return spectrogram

def plot_spectrogram(spectrogram, ax):
  if len(spectrogram.shape) > 2:
    assert len(spectrogram.shape) == 3
    spectrogram = np.squeeze(spectrogram, axis=-1)
  # Convert the frequencies to log scale and transpose, so that the time is
  # represented on the x-axis (columns).
  # Add an epsilon to avoid taking a log of zero.
  log_spec = np.log(spectrogram.T + np.finfo(float).eps)
  height = log_spec.shape[0]
  width = log_spec.shape[1]
  X = np.linspace(0, np.size(spectrogram), num=width, dtype=int)

  N = height
  n = np.arange(N)
  T = N / samplingrate
  freq = n / T
  Y = freq/2000
  ax.pcolormesh(X, Y, log_spec, norm=norm)
  print(f'X={len(X)} Y={len(Y)}')

def get_numpy_spec(waveform):
    spectrogram_numpy = []
    for idx in range(math.ceil(len(waveform)/frame_step) - 1):
        signal_chunk = waveform[idx * frame_step : idx * frame_step + window_size]
        signal_chunk = signal_chunk - np.mean(signal_chunk)
        signal_chunk = signal_chunk * np.hanning(window_size)
        X = np.abs(np.fft.fft(signal_chunk))
        spectrogram_numpy.append(np.array(X[:frame_step+1]))

    spectrogram_numpy = np.array(spectrogram_numpy)
    return spectrogram_numpy

def plot_numpy_spec(spectrogram_numpy, ax):
    log_spec = np.log(spectrogram_numpy.T + np.finfo(float).eps)
    height = log_spec.shape[0]
    width = log_spec.shape[1]
    X = np.linspace(0, np.size(spectrogram_numpy), num=width, dtype=int)

    N = height
    n = np.arange(N)
    T = N / samplingrate
    freq = n / T
    Y = freq/2000
    mesh = ax.pcolormesh(X, Y, log_spec, norm=norm)
    print(f'X={len(X)} Y={len(Y)}')
    return mesh

#testfile = os.listdir('../ml/data/mini_speech_commands/test/yes/')[0]
#x = tf.io.read_file(str(os.path.join('../ml/data/mini_speech_commands/test/yes/', testfile)))
x = tf.io.read_file(str('input.wav'))
x, sample_rate = tf.audio.decode_wav(x, desired_channels=1, desired_samples=16000,)
x = tf.squeeze(x, axis=-1)
waveform = x
waveform = 2 * (waveform - np.min(waveform)) / (np.max(waveform) - np.min(waveform)) - 1
waveform = waveform - np.mean(waveform)

spectrogram_stft = get_spectrogram(waveform)
spectrogram_cmsis = spectrogram_stft

# Using tensorflow built-in
fig, ax = plt.subplots(1, 3, figsize=(10,8))
ax[0].set_title('tf.signal.stft')
ax[0].set_xlabel('Samples (Time)')
ax[0].set_ylabel('Frequency [kHz]')
plot_spectrogram(spectrogram_stft, ax[0])

# Using numpy as manual window
ax[1].set_title('numpy.fft.fft')
ax[1].set_xlabel('Samples (Time)')
spectrogram_numpy = get_numpy_spec(waveform)
mesh = plot_numpy_spec(spectrogram_numpy, ax[1])

## On the microcontroller
ax[2].set_title('CMSIS-DSP (on STM32)')
ax[2].set_xlabel('Samples (Time)')

data = []
with open('data.txt', 'r') as f:
    for line in f:
        data.append(float(line.strip()))

spectrogram_cmsis = []
for i in range(124):
    # The result of an N-point FFT is (N+1) points
    data_window = data[i * (frame_step + 1): i * (frame_step + 1) + window_size]
    spectrogram_cmsis.append(np.array(data_window[:frame_step+1]))
spectrogram_cmsis = np.array(spectrogram_cmsis)

mesh = plot_numpy_spec(spectrogram_cmsis, ax[2])

fig.suptitle('Spectrograms Of Spoken \"Yes\" Keyword Calculated By Different Methods and Platforms')
cbar = fig.colorbar(mesh, ax=ax.ravel().tolist(), orientation='horizontal', pad=0.1, shrink=0.3)
cbar.set_label('Magnitude')
plt.show()
