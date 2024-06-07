cmake_minimum_required(VERSION 3.8)
project(stm32-speech-recognition C ASM CXX)

# Bare-metal project
set(CMAKE_SYSTEM_NAME Generic)

# Compiler settings
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-c++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(OBJCOPY arm-none-eabi-objcopy)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_SYSTEM_PROCESSOR cortex-m4)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

# For clangd and YouCompleteMe
set(CMAKE_EXPORT_COMPILE_COMMANDS ON )

# Cortex M4 with FPU
add_compile_options(-mcpu=${CMAKE_SYSTEM_PROCESSOR})
add_compile_options(-mfpu=fpv4-sp-d16 -mfloat-abi=hard)
add_compile_options(-mthumb)

# Reduced Libc
add_compile_options(--specs=nano.specs --specs=nosys.specs)

# No position-independence/GOT
add_compile_options(-fno-PIC)

# TODO: check whether this reduces code size
add_compile_options(-ffunction-sections -fdata-sections)

# Warnings/Errors
add_compile_options(-Wall -Wunused-result -Werror)

# Optimizations
add_compile_options(-O2 -ggdb)

# Defines
add_compile_options(-DSTM32L475xx -DDEBUG)

# Security/Debugging
add_compile_options(-fstack-protector-all)

# This is a workaround for Vim YCM plugin not being able to locate system
# headers. It should have no effect on the build.
include_directories(/opt/gcc-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/13.2.1/include)
include_directories(/opt/gcc-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/13.2.1/include-fixed)
include_directories(/opt/gcc-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/13.2.1/../../../../arm-none-eabi/include)

# Hardware/HAL includes
include_directories(include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/STM32CubeL4/Drivers/CMSIS/Device/ST/STM32L4xx/Include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/STM32CubeL4/Drivers/CMSIS/Core/Include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/STM32CubeL4/Drivers/STM32L4xx_HAL_Driver/Inc)
include_directories(${CMAKE_SOURCE_DIR}/third_party/STM32CubeL4/Drivers/BSP/B-L475E-IOT01)

# TFlite includes
include_directories(include/models)
include_directories(${CMAKE_SOURCE_DIR}/third_party/tflite-micro)
include_directories(${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/tools/make/downloads/flatbuffers/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/tools/make/downloads/kissfft)
include_directories(${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/tools/make/downloads/flatbuffers/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/tools/make/downloads/ruy)
include_directories(${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/tools/make/downloads/gemmlowp)

# Libcrc32 includes
include_directories(${CMAKE_SOURCE_DIR}/third_party/libcrc/include)

# Link
FILE(GLOB linker_script ld/*.ld)
add_link_options(-T ${linker_script})
add_link_options(-mcpu=${CMAKE_SYSTEM_PROCESSOR})
add_link_options(-mfloat-abi=hard)

# HAL Library
FILE(GLOB hal_srcs third_party/STM32CubeL4/Drivers/STM32L4xx_HAL_Driver/Src/*.c)
add_library(hal STATIC ${hal_srcs})

# TFLite Library
FILE(GLOB tflm_srcs
	${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/*.cc
	${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/core/c/*.cc
	${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/core/api/*.cc
	${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/*.cc
	${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/tflite_bridge/*.cc
	${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/arena_allocator/*.cc
	${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/memory_planner/*.cc
	${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/*.cc
	${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/kernels/internal/reference/*.cc
	${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/kernels/internal/*.cc
	${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/kernels/*.cc
	${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/schema/*.cc
	${CMAKE_SOURCE_DIR}/third_party/tflite-micro/third_party/kissfft/*.c
	${CMAKE_SOURCE_DIR}/third_party/tflite-micro/third_party/kissfft/tools/*.c
)
# Remove testing functionality from tflm library
list(FILTER tflm_srcs EXCLUDE REGEX ".*_test\.c+")

# Download dependencies for tflm
add_custom_command(OUTPUT
    ${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/tools/make/downloads
    COMMAND
    make -f ./tensorflow/lite/micro/tools/make/Makefile third_party_downloads
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/third_party/tflite-micro
)
add_library(tflm
    STATIC ${tflm_srcs}
    ${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/tools/make/downloads
)

# This target generates the CRC32 table required for libcrc32
# This will NOT be cross-compiled to work around compilation errors,
# as the generated header is architecture agnostic.
add_custom_target(gentab32 COMMAND make -C
	${CMAKE_SOURCE_DIR}/third_party/libcrc tab/gentab32.inc
)

# This creates the cmake library representation of libcrc
add_library(crc32 STATIC ${CMAKE_SOURCE_DIR}/third_party/libcrc/src/crc32.c)
add_dependencies(crc32 gentab32)

# Project sources
FILE(GLOB demo_srcs src/*.c
	src/*.cc
	src/models/*.cc
	third_party/STM32CubeL4/Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/gcc/startup_stm32l475xx.s
	third_party/STM32CubeL4/Drivers/BSP/B-L475E-IOT01/stm32l475e_iot01.c
)

add_executable(demo.elf ${demo_srcs})
target_link_libraries(demo.elf hal tflm crc32)

# st-util wants a binary-only format, not an ELF
add_custom_target(bin ALL DEPENDS demo.elf
	COMMAND ${OBJCOPY} -O binary demo.elf demo.bin
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# Upload using st-util
add_custom_target(flash DEPENDS bin
	COMMAND st-flash --reset write demo.bin 0x8000000
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

