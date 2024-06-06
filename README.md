# Speech Recognition on STM32 using Machine Learning
![build status](https://github.com/stgloorious/stm32-speech-recognition/actions/workflows/cmake-single-platform.yml/badge.svg)

[ML on MCU](https://www.vvz.ethz.ch/Vorlesungsverzeichnis/lerneinheit.view?semkez=2024S&ansicht=KATALOGDATEN&lerneinheitId=176625&lang=en) Demo Project

This uses the [TensorFlow Lite for Microcontrollers](https://github.com/tensorflow/tflite-micro/)
framework to perform simple keyword recognition on an STM32L475VGT
B-L745E-IOT01A2 development board.

## Dependencies
You only need some essentials and the `arm-none-eabi` toolchain.
On Ubuntu 23.10 and similar this should be enough to build the project:

~~~
sudo apt-get install build-essential cmake gcc-arm-none-eabi python3-numpy python3-pil unzip
~~~

Make sure you clone the repository with `--recursive`, as it contains submodules.

## Model Training
Create a virtual environment and install the python dependencies
~~~
cd ml
python -m venv venv
source venv/bin/activate
pip install -r requirements.txt
~~~

The model can be trained by running `train.py`. This will also copy it
to `src/models`, where it will be compiled into the firmware in the next
step.

~~~
python train.py
~~~

## Build
With the model being trained, you can proceed to build the code.
~~~
cd stm32-speech-recognition
cmake -B build && make -C build
~~~

## Upload
To upload the compiled binary to the board, you can either use
[st-util](https://github.com/stlink-org/stlink), STM32CubeIDE,
or any other SWD programmer (e.g., SEGGER j-link with Ozone).
