# stm32-nucleo-l432kc-demo
Simple Demo project using the STM32 NUCLEO-L432KC using the ST HAL library and 
the CMake build system (no STM32CubeIDE needed).

## Dependencies
To upload the binary to the board you need 
the [STM32 programming toolset](https://github.com/stlink-org/stlink).
You will also need a [suitable GCC compiler](https://developer.arm.com/downloads/-/gnu-rm)
for the ARM architecture and must have the basic build systems installed.

## Build
~~~
cmake -B build -G Ninja && ninja -C build
~~~

## Upload
~~~
cd build
ninja flash
~~~
