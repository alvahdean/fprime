####
# freertos/Platform/CMakeLists.txt:
#
# This file defines a configuration module for FreeRTOS platform types.
####
register_fprime_config(
        FreeRTOSPlatformTypes
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/PlatformTypes.fpp"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/PlatformTypes.h"
    INTERFACE # No compilable outputs produced
    BASE_CONFIG
)
