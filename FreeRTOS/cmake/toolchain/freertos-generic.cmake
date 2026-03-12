####
# File: toolchain/freertos-generic.cmake
#
# Native-host compiler toolchain that targets the FreeRTOS platform
# selection in F´. Intended for generic OSAL build/test bring-up.
####
set(CMAKE_SYSTEM_NAME Generic)
set(FPRIME_PLATFORM FreeRTOS)

# Standard program names
find_program(CMAKE_C_COMPILER NAMES cc gcc clang)
find_program(CMAKE_CXX_COMPILER NAMES c++ g++ clang++)

