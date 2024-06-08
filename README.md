# Speech Recognition on STM32 using Machine Learning
![build status](https://github.com/stgloorious/stm32-speech-recognition/actions/workflows/cmake-single-platform.yml/badge.svg)

[ML on MCU](https://www.vvz.ethz.ch/Vorlesungsverzeichnis/lerneinheit.view?semkez=2024S&ansicht=KATALOGDATEN&lerneinheitId=176625&lang=en) Demo Project

This uses the [TensorFlow Lite for Microcontrollers](https://github.com/tensorflow/tflite-micro/)
framework to perform simple keyword recognition of "YES" and "NO" on an STM32L475VGT
B-L745E-IOT01A2 development board.

:warning: Make sure you clone the repository with `--recursive`, as it contains submodules.

## Dependencies
You only need some essentials and the `arm-none-eabi` toolchain.
On Ubuntu 23.10 and similar this should be enough to build the project:

~~~
sudo apt-get update
sudo apt-get install build-essential cmake gcc-arm-none-eabi python3-numpy python3-pil unzip
~~~

Making it work on other Linux distros is possible, running it on Windows
requires major changes because a lot of Linux-specifics are hard-coded,
and is probably not worth the trouble.

## Model Training
Create a virtual environment and install the python dependencies
~~~
cd ml
python -m venv venv
source venv/bin/activate
pip install -r requirements.txt
~~~

The model can be trained by running `train.py`, which will also download the
dataset and split it into train, test and validation sets.
The model will only be trained if `model.keras` does not exist already,
so delete that to force retraining.

~~~
python train.py
~~~

Once the model is trained, it is automatically copied to `src/models`,
where it will be compiled into the firmware in the next
step.

## Build
With the model trained, you can proceed to build the code.

~~~
cd stm32-speech-recognition
cmake -B build && make -C build
~~~

## Upload
To upload the compiled binary to the board, you can either use
[st-util](https://github.com/stlink-org/stlink), STM32CubeIDE,
or any other SWD programmer (e.g., SEGGER j-link with Ozone).

## Evaluation
To evaluate the performance of the model running on the microcontroller,
there are some helper scripts in `tools`. These scripts automatically
send waveforms from the test set to the STM32 over UART, where the inference
is triggered. Once it completes, the prediction is sent back and stored
in `log.csv` along with some other helpful information.
