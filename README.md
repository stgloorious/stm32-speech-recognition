# stm32-nucleo-l432kc-demo
![build status](https://github.com/stgloorious/stm32-nucleo-l432kc-demo/actions/workflows/cmake-single-platform.yml/badge.svg)

Demo project showing my setup for STM32 microcontrollers without STM32CubeIDE.
This demo targets the STM32 NUCLEO-L432KC board (STM32L432KC).

 - Modular setup: HAL drivers are fetched from upstream repositories and are separate from user code.
 - [newlib](https://sourceware.org/newlib/) as libc implementation, built with FPU support
 - UART debug console as `stdout`/`stderr`: Support for `printf()`, assertions, exceptions etc.
 - CMake build system

Example Output:
 ~~~
STM32 NUCLEO-K432KC Demo Application Version 1.0
Build date: Nov 13 2023 20:39:27
GCC 13.2.0 Newlib 4.3.0

Hello World!
Hello World!
Hello World!
Hello World!
Hello World!
Hello World!
 ~~~

## Dependencies
### Toolchain
I built the toolchain with [crosstool-NG](https://crosstool-ng.github.io/).
The main reason for this is that I had problems selecting a `libc.a` with
hard-float support using the standard
[arm-none-eabi](https://developer.arm.com/Tools%20and%20Software/GNU%20Toolchain).
The STM32F432KC Cortex-M4 does have hardware support for `float` (single-precision)
arithmetic, so it makes sense to use the right toolchain/libc.

1. [Download crosstool-NG](https://crosstool-ng.github.io/download/).
2. Copy `toolchain.config` to `.config`
3. Run `ct-ng oldconfig` to update, `ct-ng menuconfig` to review and `ct-ng build` to build the toolchain.
4. After successful build, make sure the binaries are in your `$PATH`

### Debugging & Loader
To upload the binary to the board I use
the [STM32 programming toolset](https://github.com/stlink-org/stlink).
To start a GDB server, I use `st-util --connect-under-reset`.
This makes the most sense when using the on-board ST-Link on the Nucleo development
board.

However, sometimes it's also nice to have a graphical debugger, for which I use
[Ozone](https://www.segger.com/products/development-tools/ozone-j-link-debugger/)
with a SEGGER j-link Plus connected over SWD.
Ozone can also directly flash the STM32L432KC, so no need for the ST-Link tools.

## Build
Make sure you cloned the whole repository including the submodules.
~~~
git clone --recursive https://github.com/stgloorious/stm32-nucleo-l432kc-demo.git
~~~

To compile, run CMake
~~~
cmake -B build && make -C build
~~~

## Upload

To flash the program to the board, run
~~~
make -C build flash
~~~

## Notes
The linker script is taken from
~~~
STM32CubeL4/Projects/NUCLEO-L432KC/Templates/STM32CubeIDE/STM32L432KCUX_FLASH.ld
~~~
It looks like only one label (end of `.text`) had to be added for newlib compatibility.

The `system_stm32l4x.c` file can also be found in `third_party/STM32CubeL4/Projects/NUCLEO-L432KC/`.

