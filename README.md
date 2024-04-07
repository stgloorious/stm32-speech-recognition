# Speech Recognition on STM32 using Machine Learning
![build status](https://github.com/stgloorious/stm32-speech-recognition/actions/workflows/cmake-single-platform.yml/badge.svg)

ML on MCU Demo Project

## Dependencies
You only need some essentials and the `arm-none-eabi` toolchain.
On Ubuntu 23.10 and similar this should be enough to build the project:

~~~
sudo apt-get install build-essential cmake gcc-arm-none-eabi python3-numpy python3-pil unzip
~~~

## Build
Make sure you clone the repository with `--recursive`, as it contains submodules.

~~~
cd stm32-speech-recognition
cmake -B build && make -C build
~~~

## Upload
To upload the compiled binary to the board, you can either use
[st-util](https://github.com/stlink-org/stlink), STM32CubeIDE,
or any other SWD programmer (e.g., SEGGER j-link with Ozone).
