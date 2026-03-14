####
# FreeRtosStm32.cmake:
#
# STM32 + FreeRTOS platform file. This platform is intended for baremetal
# cross-compilation and therefore does not enable POSIX or socket features.
####

set(FPRIME_USE_POSIX OFF)
set(FPRIME_HAS_SOCKETS OFF)

# Embedded targets generally start with stubbed drivers until board-specific
# drivers are supplied by the deployment.
if (NOT DEFINED FPRIME_USE_STUBBED_DRIVERS)
    set(FPRIME_USE_STUBBED_DRIVERS ON)
endif()

# Platform file selection is independent from toolchain selection, but this
# platform expects the STM32 ARM GCC toolchain.
if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    get_filename_component(
        FreeRTOS_STM32_DEFAULT_TOOLCHAIN
        "${CMAKE_CURRENT_LIST_DIR}/../toolchain/arm-gcc-stm32.cmake"
        ABSOLUTE
    )
    message(WARNING
        "[F-PRIME] PlatformFreeRtosStm32 platform selected without an explicit toolchain. "
        "Use -DCMAKE_TOOLCHAIN_FILE=${FreeRTOS_STM32_DEFAULT_TOOLCHAIN}."
    )
endif()

# Register FreeRtosStm32 base config and implementation choices.
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/FreeRtosStm32")

# FreeRtosStm32Config is registered in FreeRtosStm32/CMakeLists.txt.
target_compile_definitions(
    FreeRtosStm32Config
    INTERFACE
        -DTGT_OS_TYPE_FREERTOS
        -D__STDC_FORMAT_MACROS
)
target_compile_options(
    FreeRtosStm32Config
    INTERFACE
        -include
        "${CMAKE_CURRENT_LIST_DIR}/FreeRtosStm32/inttypes_compat.h"
        -Wno-error=type-limits
        -Wno-error=conversion
)
