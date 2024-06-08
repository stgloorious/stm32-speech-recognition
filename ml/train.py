#!/usr/bin/env python

# Copyright 2024 Stefan Gloor
# Based on https://www.tensorflow.org/tutorials/audio/simple_audio:
# Copyright 2020 The TensorFlow Authors.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import pathlib
import shutil

import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
import tensorflow as tf

from tensorflow.keras import layers
from tensorflow.keras import models
from sklearn.model_selection import train_test_split

# Set the seed value for experiment reproducibility.
seed = 42
tf.random.set_seed(seed)
np.random.seed(seed)

DATASET_PATH = 'data/mini_speech_commands'

data_dir = pathlib.Path(DATASET_PATH)
if not data_dir.exists():
    tf.keras.utils.get_file(
        'mini_speech_commands.zip',
        origin="http://storage.googleapis.com/download.tensorflow.org/data/mini_speech_commands.zip",
        extract=True,
        cache_dir='.', cache_subdir='data')
    # Delete the unwanted parts of the dataset
    shutil.rmtree(os.path.join(data_dir, 'left'))
    shutil.rmtree(os.path.join(data_dir, 'right'))
    shutil.rmtree(os.path.join(data_dir, 'up'))
    shutil.rmtree(os.path.join(data_dir, 'down'))
    shutil.rmtree(os.path.join(data_dir, 'go'))
    shutil.rmtree(os.path.join(data_dir, 'stop'))

    for data in ['yes', 'no']:
        curr_dir = os.path.join(data_dir, data)
        all_files = [os.path.join(curr_dir, fn) for fn in os.listdir(curr_dir) if fn.endswith('.wav')]

        # Split into 75% train
        train_files, val_files = train_test_split(all_files, test_size=0.25, random_state=0)

        # Split the remaining 25% into 12.5% validation and 12.5% test
        test_files, val_files = train_test_split(val_files, test_size=0.5, random_state=0)

        print(f'Number of test files: {len(test_files)}')
        print(f'Number of train files: {len(train_files)}')
        print(f'Number of val files: {len(val_files)}')

        if not os.path.exists(os.path.join(os.path.join(data_dir, 'train'), data)):
            os.makedirs(os.path.join(os.path.join(os.path.join(data_dir, 'train'), data)))
        if not os.path.exists(os.path.join(os.path.join(data_dir, 'test'), data)):
            os.makedirs(os.path.join(os.path.join(os.path.join(data_dir, 'test'), data)))
        if not os.path.exists(os.path.join(os.path.join(data_dir, 'val'), data)):
            os.makedirs(os.path.join(os.path.join(os.path.join(data_dir, 'val'), data)))

        for f in test_files:
            os.rename(f, os.path.join(os.path.join(os.path.join(data_dir, 'test'), data), os.path.basename(f)))
        for f in train_files:
            os.rename(f, os.path.join(os.path.join(os.path.join(data_dir, 'train'), data), os.path.basename(f)))
        for f in val_files:
            os.rename(f, os.path.join(os.path.join(os.path.join(data_dir, 'val'), data), os.path.basename(f)))

train_ds = tf.keras.utils.audio_dataset_from_directory(
    directory=os.path.join(os.path.join(data_dir, 'train')),
    output_sequence_length=16000)

test_ds = tf.keras.utils.audio_dataset_from_directory(
    directory=os.path.join(os.path.join(data_dir, 'test')),
    output_sequence_length=16000)

val_ds = tf.keras.utils.audio_dataset_from_directory(
    directory=os.path.join(os.path.join(data_dir, 'val')),
    output_sequence_length=16000)

label_names = np.array(train_ds.class_names)
print("label names:", label_names)

# This dataset only contains single channel audio, so use the `tf.squeeze`
# function to drop the extra axis:
def squeeze(audio, labels):
  audio = tf.squeeze(audio, axis=-1)
  return audio, labels

train_ds = train_ds.map(squeeze, tf.data.AUTOTUNE)
val_ds = val_ds.map(squeeze, tf.data.AUTOTUNE)

# Split the validation set again
test_ds = val_ds.shard(num_shards=2, index=0)
val_ds = val_ds.shard(num_shards=2, index=1)

# Apply STFT to waveforms to get spectrogram
def get_spectrogram(waveform):
  # Convert the waveform to a spectrogram via a STFT.
  spectrogram = tf.signal.stft(waveform, frame_length=255, frame_step=128)
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
  Y = range(height)
  ax.pcolormesh(X, Y, log_spec)

# Convert the waveforms to spectograms
def make_spec_ds(ds):
  return ds.map(
      map_func=lambda audio,label: (get_spectrogram(audio), label),
      num_parallel_calls=tf.data.AUTOTUNE)

train_spectrogram_ds = make_spec_ds(train_ds)
val_spectrogram_ds = make_spec_ds(val_ds)
test_spectrogram_ds = make_spec_ds(test_ds)

train_spectrogram_ds = train_spectrogram_ds.cache().shuffle(10000).prefetch(tf.data.AUTOTUNE)
val_spectrogram_ds = val_spectrogram_ds.cache().prefetch(tf.data.AUTOTUNE)
test_spectrogram_ds = test_spectrogram_ds.cache().prefetch(tf.data.AUTOTUNE)

num_labels = len(label_names)
print(train_spectrogram_ds)

input_shape = train_spectrogram_ds.element_spec[0].shape[1:]
print(f'Input shape={input_shape}')



if not os.path.exists('model.keras'):
    # Instantiate the `tf.keras.layers.Normalization` layer.
    norm_layer = layers.Normalization()
    # Fit the state of the layer to the spectrograms
    # with `Normalization.adapt`.
    norm_layer.adapt(data=train_spectrogram_ds.map(map_func=lambda spec, label: spec))

    model = models.Sequential([
        layers.Input(shape=input_shape),
        # Downsample the input.
        layers.Resizing(32, 32),
        # Normalize.
        norm_layer,
        layers.Conv2D(16, 3, activation='relu'),
        layers.MaxPooling2D(),
        layers.Dropout(0.25),
        layers.Flatten(),
        layers.Dense(8, activation='relu'),
        layers.Dropout(0.5),
        layers.Dense(num_labels),
    ])

    model.summary()

    model.compile(
        optimizer=tf.keras.optimizers.Adam(),
        loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True),
        metrics=['accuracy'],
    )

    EPOCHS = 10
    history = model.fit(
        train_spectrogram_ds,
        validation_data=val_spectrogram_ds,
        epochs=EPOCHS,
        callbacks=tf.keras.callbacks.EarlyStopping(verbose=1, patience=2),
    )
    model.save('model.keras')

    def representative_dataset():
        for input_value, _ in train_spectrogram_ds.take(100):
            yield [input_value]

    # Convert the model.
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    converter.representative_dataset = representative_dataset
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    converter.inference_input_type = tf.uint8
    converter.inference_output_type = tf.uint8
    tflite_model = converter.convert()

    # Save the model.
    with open('model.tflite', 'wb') as f:
      f.write(tflite_model)

    # Convert the model to a c array
    os.system('xxd -i model.tflite > model.cc')
    os.system('sed -i \'s/unsigned char/const unsigned char/\' model.cc')
    os.system('sed -i \'1i #include <model_tflite.h>\n\' model.cc')

    if not os.path.exists('../src/models'):
        os.makedirs('../src/models')
    shutil.copyfile('model.cc', '../src/models/model.cc')

    #metrics = history.history
    #plt.figure(figsize=(16,6))
    #plt.subplot(1,2,1)
    #plt.plot(history.epoch, metrics['loss'], metrics['val_loss'])
    #plt.legend(['loss', 'val_loss'])
    #plt.ylim([0, max(plt.ylim())])
    #plt.xlabel('Epoch')
    #plt.ylabel('Loss [CrossEntropy]')

    #plt.subplot(1,2,2)
    #plt.plot(history.epoch, 100*np.array(metrics['accuracy']), 100*np.array(metrics['val_accuracy']))
    #plt.legend(['accuracy', 'val_accuracy'])
    #plt.ylim([0, 100])
    #plt.xlabel('Epoch')
    #plt.ylabel('Accuracy [%]')

    # ## Evaluate the model performance
    model.evaluate(test_spectrogram_ds, return_dict=True)

    # ### Display a confusion matrix
    y_pred = model.predict(test_spectrogram_ds)
    y_pred = tf.argmax(y_pred, axis=1)
    y_true = tf.concat(list(test_spectrogram_ds.map(lambda s,lab: lab)), axis=0)
    confusion_mtx = tf.math.confusion_matrix(y_true, y_pred)
    plt.figure(figsize=(10, 8))
    sns.heatmap(confusion_mtx,
                xticklabels=label_names,
                yticklabels=label_names,
                annot=True, fmt='g')
    plt.xlabel('Prediction')
    plt.ylabel('Label')
    plt.show()

else:
    model = tf.keras.models.load_model('model.keras')

# ## Run inference on an audio file

testfile = os.listdir(data_dir/'test/yes/')[0]
x = tf.io.read_file(str(os.path.join(data_dir/'test/yes/', testfile)))
x, sample_rate = tf.audio.decode_wav(x, desired_channels=1, desired_samples=16000,)
x = tf.squeeze(x, axis=-1)
waveform = x
x = get_spectrogram(x)
x = x[tf.newaxis,...]

prediction = model(x)
values = tf.nn.softmax(prediction[0])
print('Prediction of float model:')
label_names = ['no', 'yes']
for i in range(len(label_names)):
    print(f'{label_names[i]}: {values[i]:.2%}')


# Load the TFLite model
with open('model.tflite', 'rb') as f:
    tflite_model = f.read()

# Initialize the TFLite interpreter
interpreter = tf.lite.Interpreter(model_content=tflite_model)
interpreter.allocate_tensors()

# Input data
input_data = np.array(x)
preprocessed_input_data = (input_data * 256).astype('uint8')

with open('sample_input.bin', 'wb') as f:
    f.write(preprocessed_input_data)

print(f'input shape {preprocessed_input_data.shape}')
print(f'input bytes {len(preprocessed_input_data)}')

# Run inference
input_data = interpreter.set_tensor(interpreter.get_input_details()[0]['index'], preprocessed_input_data)
interpreter.invoke()
output_data = interpreter.get_tensor(interpreter.get_output_details()[0]['index'])
print(f'output shape {output_data.shape}')
print(f'output bytes {len(output_data)}')
print(f'Output: {output_data}')
