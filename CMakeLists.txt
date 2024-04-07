cmake_minimum_required(VERSION 3.15)
project(stm32-speech-recognition C ASM)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(OBJCOPY arm-none-eabi-objcopy)

set(CMAKE_SYSTEM_PROCESSOR cortex-m4)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

add_compile_options(-mcpu=${CMAKE_SYSTEM_PROCESSOR})
add_compile_options(-mthumb)
add_compile_options(-mfpu=fpv4-sp-d16 -mfloat-abi=hard)
add_compile_options(--specs=nano.specs --specs=nosys.specs)
add_compile_options(-DSTM32L475xx -DDEBUG)
add_compile_options(-fno-PIC)
add_compile_options(-Wall -Wunused-result -Werror -O0 -ggdb)
include_directories(include)
include_directories(third_party/STM32CubeL4/Drivers/CMSIS/Device/ST/STM32L4xx/Include)
include_directories(third_party/STM32CubeL4/Drivers/CMSIS/Core/Include)
include_directories(third_party/STM32CubeL4/Drivers/STM32L4xx_HAL_Driver/Inc)
include_directories(third_party/STM32CubeL4/Drivers/BSP/B-L475E-IOT01)

FILE(GLOB linker_script ld/*.ld)
add_link_options(-T ${linker_script})
add_link_options(-mcpu=${CMAKE_SYSTEM_PROCESSOR})
add_link_options(-mfloat-abi=hard)

FILE(GLOB hal_srcs third_party/STM32CubeL4/Drivers/STM32L4xx_HAL_Driver/Src/*.c)
add_library(hal STATIC ${hal_srcs})

FILE(GLOB demo_srcs src/*.c
    third_party/STM32CubeL4/Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/gcc/startup_stm32l475xx.s
    third_party/STM32CubeL4/Drivers/BSP/B-L475E-IOT01/stm32l475e_iot01.c
)

add_executable(demo.elf ${demo_srcs})
target_link_libraries(demo.elf hal)

add_custom_target(bin ALL DEPENDS demo.elf
    COMMAND ${OBJCOPY} -O binary demo.elf demo.bin
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_custom_target(flash DEPENDS bin
    COMMAND st-flash --reset write demo.bin 0x8000000
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
