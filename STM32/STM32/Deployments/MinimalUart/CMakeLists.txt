####
# 'STM32MinimalUart' Deployment:
#
# Minimal STM32 deployment for validating the STM32 UART driver with
# F' GDS-compatible FPrime framing on a UART link.
####

cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.26)
project(STM32MinimalUart VERSION 1.0.0 LANGUAGES C CXX ASM)

set(FPRIME_INCLUDE_FRAMEWORK_CODE ON)
find_package(FPrime REQUIRED PATHS "${CMAKE_CURRENT_LIST_DIR}/../../../..")

add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/config")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Top")

# Defend against leaked parser state from parent CMake scopes.
unset(CURRENT_LIST_NAME)
unset(LIST_CHOOSES_IMPLEMENTATIONS)
unset(LIST_REQUIRES_IMPLEMENTATIONS)
unset(INTERNAL_CHOOSES_IMPLEMENTATIONS)
unset(INTERNAL_REQUIRES_IMPLEMENTATIONS)

# Work around stray boolean sentinel emitted into base implementations by
# sub-build metadata generation.
get_property(_base_chosen_impls GLOBAL PROPERTY FPRIME_BASE_CHOSEN_IMPLEMENTATIONS)
if (_base_chosen_impls)
    list(REMOVE_ITEM _base_chosen_impls TRUE)
    set_property(GLOBAL PROPERTY FPRIME_BASE_CHOSEN_IMPLEMENTATIONS "${_base_chosen_impls}")
endif()

register_fprime_deployment(
    STM32MinimalUart
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/Main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/startup/syscalls_stm32.c"
        "${CMAKE_CURRENT_LIST_DIR}/startup/system_stm32f4xx.c"
        "${CMAKE_CURRENT_LIST_DIR}/startup/startup_stm32f405xx.s"
    DEPENDS
        STM32_Deployments_MinimalUart_Top
        STM32_Drv_STM32UartDriver
        STM32_Subtopologies_STM32CdhCore
        STM32_Subtopologies_STM32ComFprime
        Svc_OsTime
)

# Allow `fprime-util build` from this deployment directory to resolve the
# directory-derived target name that fprime-util selects by default.
add_custom_target(STM32_Deployments_MinimalUart DEPENDS STM32MinimalUart)

set(STM32_MINIMAL_UART_LINKER_SCRIPT
    "${CMAKE_CURRENT_LIST_DIR}/ld/stm32f405.ld"
    CACHE FILEPATH
    "Linker script for the STM32MinimalUart deployment"
)

set_target_properties(STM32MinimalUart PROPERTIES SUFFIX ".elf")

target_compile_options(
    STM32MinimalUart
    PRIVATE
        -ffunction-sections
        -fdata-sections
)

target_link_options(
    STM32MinimalUart
    PRIVATE
        "-T${STM32_MINIMAL_UART_LINKER_SCRIPT}"
        "--specs=nano.specs"
        "--specs=nosys.specs"
        "-Wl,--gc-sections"
        "-Wl,--print-memory-usage"
        "-Wl,-Map,$<TARGET_FILE_DIR:STM32MinimalUart>/$<TARGET_FILE_BASE_NAME:STM32MinimalUart>.map"
)

if (CMAKE_OBJCOPY)
    add_custom_command(
        TARGET STM32MinimalUart
        POST_BUILD
        COMMAND "${CMAKE_OBJCOPY}" -O binary
                "$<TARGET_FILE:STM32MinimalUart>"
                "$<TARGET_FILE_DIR:STM32MinimalUart>/$<TARGET_FILE_BASE_NAME:STM32MinimalUart>.bin"
        COMMAND "${CMAKE_OBJCOPY}" -O ihex
                "$<TARGET_FILE:STM32MinimalUart>"
                "$<TARGET_FILE_DIR:STM32MinimalUart>/$<TARGET_FILE_BASE_NAME:STM32MinimalUart>.hex"
        VERBATIM
    )
endif()

set(STM32_MINIMAL_UART_FLASH_ADDRESS
    0x08000000
    CACHE STRING
    "Flash base address for STM32MinimalUart"
)

find_program(STM32_ST_FLASH_TOOL st-flash)
if (STM32_ST_FLASH_TOOL)
    add_custom_target(
        STM32MinimalUart_flash_stlink
        COMMAND
            "${STM32_ST_FLASH_TOOL}" --reset write
            "$<TARGET_FILE_DIR:STM32MinimalUart>/$<TARGET_FILE_BASE_NAME:STM32MinimalUart>.bin"
            "${STM32_MINIMAL_UART_FLASH_ADDRESS}"
        DEPENDS STM32MinimalUart
        USES_TERMINAL
        COMMENT "Flashing STM32MinimalUart.bin with st-flash"
    )
endif()

find_program(STM32_OPENOCD_TOOL openocd)
if (STM32_OPENOCD_TOOL)
    set(STM32_MINIMAL_UART_OPENOCD_INTERFACE
        "interface/stlink.cfg"
        CACHE STRING
        "OpenOCD interface config for STM32MinimalUart flashing"
    )
    set(STM32_MINIMAL_UART_OPENOCD_TARGET
        "target/stm32f4x.cfg"
        CACHE STRING
        "OpenOCD target config for STM32MinimalUart flashing"
    )
    add_custom_target(
        STM32MinimalUart_flash_openocd
        COMMAND
            "${STM32_OPENOCD_TOOL}"
            -f "${STM32_MINIMAL_UART_OPENOCD_INTERFACE}"
            -f "${STM32_MINIMAL_UART_OPENOCD_TARGET}"
            -c "program $<TARGET_FILE:STM32MinimalUart> verify reset exit"
        DEPENDS STM32MinimalUart
        USES_TERMINAL
        COMMENT "Flashing STM32MinimalUart.elf with OpenOCD"
    )
endif()
