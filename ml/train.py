#!/usr/bin/env python

import os
import datasets
import tensorflow as tf
import scipy
import numpy as np
import matplotlib.pyplot as plt

download_dir = '../third_party/dataset'
if not os.path.exists(download_dir):
    os.mkdir(download_dir)

datasets.config.DOWNLOADED_DATASETS_PATH = download_dir

# Load a dataset and print the first example in the training set
dataset = datasets.load_dataset('speech_commands', 'v0.02')

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
  Y = range(height)
  ax.pcolormesh(X, Y, log_spec)

def get_spectrogram(waveform):
  # Convert the waveform to a spectrogram via a STFT.
  spectrogram = tf.signal.stft(
      waveform, frame_length=255, frame_step=128)
  # Obtain the magnitude of the STFT.
  spectrogram = tf.abs(spectrogram)
  # Add a `channels` dimension, so that the spectrogram can be used
  # as image-like input data with convolution layers (which expect
  # shape (`batch_size`, `height`, `width`, `channels`).
  spectrogram = spectrogram[..., tf.newaxis]
  return spectrogram

fig, axes = plt.subplots(5, figsize=(12, 8))
for i in range(5):
    audio = dataset['train'][i]['audio']['array']
    #sampling_rate = dataset['train'][i]['audio']['sampling_rate']
    spectrogram = get_spectrogram(audio)
    plot_spectrogram(spectrogram.numpy(), axes[i])

plt.show()
