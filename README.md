# Speech Recognition on STM32 using Machine Learning
![build status](https://github.com/stgloorious/stm32-speech-recognition/actions/workflows/cmake-single-platform.yml/badge.svg)

ML on MCU Demo Project

## Dependencies
You only need a recent CMake and the `arm-none-eabi` toolchain.
On recent Ubuntu this should be enough to build the project:

~~~
sudo apt-get install gcc-arm-none-eabi
~~~

## Build
~~~
cmake -B build && make -C build
~~~

## Upload
To upload the compiled binary to the board, you can either use
[st-util](https://github.com/stlink-org/stlink), STM32CubeIDE,
or any other SWD programmer (e.g., SEGGER j-link with Ozone).
