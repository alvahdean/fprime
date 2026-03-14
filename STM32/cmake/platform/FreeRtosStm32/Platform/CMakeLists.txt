####
# FreeRtosStm32/Platform/CMakeLists.txt:
#
# Platform types for STM32 + FreeRTOS targets.
####

register_fprime_config(
        FreeRtosStm32PlatformTypes
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/PlatformTypes.fpp"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/PlatformTypes.h"
    INTERFACE # No compilable outputs produced
    BASE_CONFIG
)
