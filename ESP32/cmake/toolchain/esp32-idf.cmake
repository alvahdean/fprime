# ESP32 toolchain for ESP-IDF based builds

set(CMAKE_SYSTEM_NAME Generic)
set(FPRIME_PLATFORM ESP32FreeRTOS)
set(FPRIME_TOOLCHAIN_NAME esp32-idf CACHE INTERNAL "F Prime toolchain name")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if(NOT DEFINED IDF_PATH)
    if(DEFINED ENV{IDF_PATH})
        set(IDF_PATH "$ENV{IDF_PATH}")
    elseif(DEFINED ENV{ESP32_IDF_PATH})
        set(IDF_PATH "$ENV{ESP32_IDF_PATH}")
    endif()
endif()

if(NOT DEFINED IDF_PATH)
    message(FATAL_ERROR "esp32-idf toolchain requires IDF_PATH or ESP32_IDF_PATH")
endif()

if(NOT CMAKE_C_COMPILER OR NOT EXISTS "${CMAKE_C_COMPILER}")
    file(GLOB_RECURSE ESP32_XTENSA_GCC_CANDIDATES
        "$ENV{IDF_TOOLS_PATH}/**/xtensa-esp32-elf-gcc"
        "$ENV{HOME}/.espressif/tools/**/xtensa-esp32-elf-gcc"
    )
    list(LENGTH ESP32_XTENSA_GCC_CANDIDATES ESP32_XTENSA_GCC_COUNT)
    if(ESP32_XTENSA_GCC_COUNT GREATER 0)
        list(GET ESP32_XTENSA_GCC_CANDIDATES 0 CMAKE_C_COMPILER)
        get_filename_component(ESP32_XTENSA_BIN_DIR "${CMAKE_C_COMPILER}" DIRECTORY)
        set(ENV{PATH} "${ESP32_XTENSA_BIN_DIR}:$ENV{PATH}")
        set(CMAKE_CXX_COMPILER "${ESP32_XTENSA_BIN_DIR}/xtensa-esp32-elf-g++" CACHE FILEPATH "" FORCE)
        set(CMAKE_ASM_COMPILER "${CMAKE_C_COMPILER}" CACHE FILEPATH "" FORCE)
        set(CMAKE_C_COMPILER "${CMAKE_C_COMPILER}" CACHE FILEPATH "" FORCE)
    endif()
endif()

set(ESP32_IDF_TOOLCHAIN "${IDF_PATH}/tools/cmake/toolchain-esp32.cmake")
if(NOT EXISTS "${ESP32_IDF_TOOLCHAIN}")
    message(FATAL_ERROR "ESP-IDF toolchain file not found: ${ESP32_IDF_TOOLCHAIN}")
endif()

include("${ESP32_IDF_TOOLCHAIN}")
