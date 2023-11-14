# Bare Metal STM32 Setup without STM32CubeIDE
![build status](https://github.com/stgloorious/stm32-nucleo-l432kc-demo/actions/workflows/cmake-single-platform.yml/badge.svg)

Demo project showing my setup for STM32 microcontrollers without STM32CubeIDE, but with the HAL drivers.
This demo targets the STM32 NUCLEO-L432KC board (STM32L432KC), which contains an ARM Cortex-M4 CPU.

 - Modular setup: HAL drivers are fetched from upstream repositories and are separate from application code.
 - [newlib](https://sourceware.org/newlib/) as libc implementation, built with FPU support.
 - UART debug console as `stdout`/`stderr`: Support for `printf()`, assertions, exceptions etc.
 - CMake build system.

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
I built a toolchain with [crosstool-NG](https://crosstool-ng.github.io/)
which was tailored to the STM32 by using `-mcpu=cortex-m4`, `-mfloat-abi=hard` and `mfpu=fpv4-sp-d16`.
The main reason for this is that I had problems linking against libc with
hard-float support using the standard
[arm-none-eabi](https://developer.arm.com/Tools%20and%20Software/GNU%20Toolchain),
as it would always link against the standard (soft float) libc, resulting
in errors like these:
~~~
<executable> uses VFP register arguments, <libc> does not
~~~
regardless of settings flags `-nostdlib`.

The STM32F432KC ARM Cortex-M4 does have hardware support for `float` (single-precision)
arithmetic, so it makes sense to use the right toolchain/libc.

To build the toolchain:

1. [Download crosstool-NG](https://crosstool-ng.github.io/download/) and install it.
2. Copy `toolchain.config` to `.config`
3. Run `ct-ng oldconfig` to update, `ct-ng menuconfig` to review and `ct-ng build` to build the toolchain.
4. After successful build, make sure the binaries are in your `$PATH`

### Debugging & Loader
To upload the final binary to the board I use
the [STM32 programming toolset](https://github.com/stlink-org/stlink).
To start a GDB server, I use `st-util --connect-under-reset`.
This makes the most sense when using the on-board ST-Link on the NUCLEO development
board.

However, sometimes it's also nice to have a more capable graphical debugger, for which I use
[Ozone](https://www.segger.com/products/development-tools/ozone-j-link-debugger/)
with a SEGGER J-Link Plus connected over SWD. I also tried enabling tracing via the SWO pin, 
but could not get it to work.
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

