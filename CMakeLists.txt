cmake_minimum_required(VERSION 3.8)
project(stm32-speech-recognition C ASM CXX)

# Bare-metal project
set(CMAKE_SYSTEM_NAME Generic)

# For CMSIS-NN
include(ExternalProject)

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

# CMSIS includes
include_directories(${CMAKE_SOURCE_DIR}/third_party/CMSIS_NN)
include_directories(${CMAKE_SOURCE_DIR}/third_party/CMSIS-DSP/Include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/CMSIS_5/CMSIS/Core/Include)

# Link
FILE(GLOB linker_script ld/*.ld)
add_link_options(-T ${linker_script})
add_link_options(-mcpu=${CMAKE_SYSTEM_PROCESSOR})
add_link_options(-mfloat-abi=hard)

# HAL Library
FILE(GLOB hal_srcs third_party/STM32CubeL4/Drivers/STM32L4xx_HAL_Driver/Src/*.c)
add_library(hal STATIC ${hal_srcs})

# CMSIS-NN
set(LIBCMSISNN_PATH ${CMAKE_BINARY_DIR}/cmsis-nn-prefix/src/cmsis-nn-build/libcmsis-nn.a)
set(LIBCMSISNN_CXXFLAGS "-mthumb -mcpu=${CMAKE_SYSTEM_PROCESSOR} \
-mfpu=fpv4-sp-d16 -mfloat-abi=hard -nostdlib -s \
-ffunction-sections -fdata-sections")

ExternalProject_Add(cmsis-nn
	SOURCE_DIR ${CMAKE_SOURCE_DIR}/third_party/CMSIS-NN
	INSTALL_COMMAND ""
	CMAKE_ARGS
	-DCMAKE_CXX_FLAGS=${LIBCMSISNN_CXXFLAGS}
	-DCMAKE_C_FLAGS=${LIBCMSISNN_CXXFLAGS}
	-DCMAKE_STATIC_LINKER_FLAGS=
	-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
	-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
)
add_library(cmsisnn SHARED IMPORTED)
set_target_properties(cmsisnn PROPERTIES IMPORTED_LOCATION ${LIBCMSISNN_PATH})

# CMSIS-DSP
set(LIBCMSISDSP_PATH
	${CMAKE_BINARY_DIR}/cmsis-dsp-prefix/src/cmsis-dsp-build/libCMSISDSP.a)
set(LIBCMSISDSP_CXXFLAGS "-mthumb -mcpu=${CMAKE_SYSTEM_PROCESSOR} \
-mfpu=fpv4-sp-d16 -mfloat-abi=hard -nostdlib \
-iquote ${CMAKE_SOURCE_DIR}/third_party/CMSIS_5/CMSIS/Core/Include \
-ffunction-sections -fdata-sections\
")

ExternalProject_Add(cmsis-dsp
	SOURCE_DIR ${CMAKE_SOURCE_DIR}/third_party/CMSIS-DSP/Source
	PATCH_COMMAND git restore . && git apply ${CMAKE_SOURCE_DIR}/third_party/patches/cmsis-dsp-remove-tables.patch
	INSTALL_COMMAND ""
	CMAKE_ARGS
	-DCMAKE_CXX_FLAGS=${LIBCMSISDSP_CXXFLAGS}
	-DCMAKE_C_FLAGS=${LIBCMSISDSP_CXXFLAGS}
	-DCMAKE_STATIC_LINKER_FLAGS=
	-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
	-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
)
add_library(cmsisdsp SHARED IMPORTED)
set_target_properties(cmsisdsp PROPERTIES IMPORTED_LOCATION ${LIBCMSISDSP_PATH})
add_dependencies(cmsisdsp cmsis-dsp)

# Project sources
FILE(GLOB demo_srcs src/*.c
	src/*.cc
	src/models/*.cc
	third_party/STM32CubeL4/Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/gcc/startup_stm32l475xx.s
	third_party/STM32CubeL4/Drivers/BSP/B-L475E-IOT01/stm32l475e_iot01.c
)

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
	${CMAKE_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/*.cc
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
# TODO fix -Wunused-variable
target_compile_options(tflm PRIVATE -DCMSIS_NN -iquote
	${CMAKE_SOURCE_DIR}/third_party/CMSIS-NN -Wno-unused-variable)

# This target generates the CRC32 table required for libcrc32
# This will NOT be cross-compiled to work around compilation errors,
# as the generated header is architecture agnostic.
add_custom_target(gentab32 COMMAND make -C
	${CMAKE_SOURCE_DIR}/third_party/libcrc tab/gentab32.inc
)

# This creates the cmake library representation of libcrc
add_library(crc32 STATIC ${CMAKE_SOURCE_DIR}/third_party/libcrc/src/crc32.c)
add_dependencies(crc32 gentab32)

add_executable(demo.elf ${demo_srcs})
target_link_libraries(demo.elf hal tflm crc32 cmsisnn cmsisdsp)

# st-util wants a binary-only format, not an ELF
add_custom_target(bin ALL DEPENDS demo.elf
	COMMAND ${OBJCOPY} -O binary demo.elf demo.bin
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# Upload using st-util
add_custom_target(flash DEPENDS bin
	COMMAND st-flash --reset write demo.bin 0x8000000
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

