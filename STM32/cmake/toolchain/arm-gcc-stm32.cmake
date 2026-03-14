####
# ARM GCC STM32 Toolchain
#
# Cross-compilation toolchain for STM32-class MCUs using GNU Arm Embedded
# (arm-none-eabi-*). This file is toolchain-only and intentionally does not
# include STM32 HAL/Cube paths.
####

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(FPRIME_PLATFORM FreeRtosStm32)

# Avoid executable link checks during toolchain probing. Link/startup details
# are deployment-specific for embedded targets.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(STM32_CPU "cortex-m4" CACHE STRING "Target Cortex-M CPU name passed to -mcpu")
set(STM32_FPU "" CACHE STRING "Target FPU name passed to -mfpu (empty to disable)")
set(STM32_FLOAT_ABI "soft" CACHE STRING "Float ABI passed to -mfloat-abi when STM32_FPU is set")
set(STM32_LINKER_SCRIPT "" CACHE FILEPATH "Optional linker script path")

set(STM32_TOOLCHAIN_PREFIX "arm-none-eabi" CACHE STRING "GNU Arm toolchain prefix")
set(STM32_TOOLCHAIN_PATH "" CACHE PATH "Optional path to toolchain root or bin directory")
set(STM32_SKIP_COMPILER_PREFIX_CHECK OFF CACHE BOOL "Allow configure to continue without arm-none-eabi-gcc/g++")

set(STM32_EXTRA_ARCH_FLAGS "" CACHE STRING "Additional target architecture flags")

set(_STM32_FIND_INPUTS)
if (STM32_TOOLCHAIN_PATH)
    list(APPEND _STM32_FIND_INPUTS PATHS "${STM32_TOOLCHAIN_PATH}" "${STM32_TOOLCHAIN_PATH}/bin")
endif()

if (NOT CMAKE_C_COMPILER)
    find_program(CMAKE_C_COMPILER NAMES "${STM32_TOOLCHAIN_PREFIX}-gcc" ${_STM32_FIND_INPUTS})
endif()
if (NOT CMAKE_CXX_COMPILER)
    find_program(CMAKE_CXX_COMPILER NAMES "${STM32_TOOLCHAIN_PREFIX}-g++" ${_STM32_FIND_INPUTS})
endif()
if (NOT CMAKE_ASM_COMPILER)
    find_program(CMAKE_ASM_COMPILER NAMES "${STM32_TOOLCHAIN_PREFIX}-gcc" ${_STM32_FIND_INPUTS})
endif()
if (NOT CMAKE_AR)
    find_program(CMAKE_AR NAMES "${STM32_TOOLCHAIN_PREFIX}-ar" ${_STM32_FIND_INPUTS})
endif()
if (NOT CMAKE_OBJCOPY)
    find_program(CMAKE_OBJCOPY NAMES "${STM32_TOOLCHAIN_PREFIX}-objcopy" ${_STM32_FIND_INPUTS})
endif()
if (NOT CMAKE_OBJDUMP)
    find_program(CMAKE_OBJDUMP NAMES "${STM32_TOOLCHAIN_PREFIX}-objdump" ${_STM32_FIND_INPUTS})
endif()

if (NOT CMAKE_C_COMPILER)
    message(WARNING
        "[F-PRIME] Could not locate ${STM32_TOOLCHAIN_PREFIX}-gcc. "
        "Set STM32_TOOLCHAIN_PATH or adjust STM32_TOOLCHAIN_PREFIX."
    )
endif()

# Host tools should come from host PATH, while target headers/libs should come
# from a cross sysroot when one is provided.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(_STM32_ARCH_FLAGS)
if (STM32_CPU)
    list(APPEND _STM32_ARCH_FLAGS "-mcpu=${STM32_CPU}" "-mthumb")
endif()
if (STM32_FPU)
    list(APPEND _STM32_ARCH_FLAGS "-mfpu=${STM32_FPU}" "-mfloat-abi=${STM32_FLOAT_ABI}")
endif()
if (STM32_EXTRA_ARCH_FLAGS)
    list(APPEND _STM32_ARCH_FLAGS "${STM32_EXTRA_ARCH_FLAGS}")
endif()
string(JOIN " " _STM32_ARCH_FLAGS_STRING ${_STM32_ARCH_FLAGS})

set(CMAKE_C_FLAGS_INIT "${_STM32_ARCH_FLAGS_STRING}")
set(CMAKE_CXX_FLAGS_INIT "${_STM32_ARCH_FLAGS_STRING}")
set(CMAKE_ASM_FLAGS_INIT "${_STM32_ARCH_FLAGS_STRING}")

if (STM32_LINKER_SCRIPT)
    set(CMAKE_EXE_LINKER_FLAGS_INIT "${_STM32_ARCH_FLAGS_STRING} -T\"${STM32_LINKER_SCRIPT}\"")
else()
    set(CMAKE_EXE_LINKER_FLAGS_INIT "${_STM32_ARCH_FLAGS_STRING}")
endif()
