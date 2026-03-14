####
# FreeRtosStm32/CMakeLists.txt:
#
# Base configuration and implementation choices for STM32 + FreeRTOS targets.
####

# PlatformTypes.h is consumed via <Platform/PlatformTypes.h> from FpConfig.h.
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Platform")

register_fprime_config(
        FreeRtosStm32Config
    INTERFACE
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/FreeRtosStm32.fpp"
    CHOOSES_IMPLEMENTATIONS
        Os_File_FreeRTOS
        Os_Console_FreeRTOS
        Os_Task_FreeRTOS
        Os_Mutex_FreeRTOS
        Os_Queue_FreeRTOS
        Os_RawTime_FreeRTOS
        Os_Cpu_FreeRTOS
        Os_Memory_FreeRTOS
        Fw_StringFormat_snprintf
    BASE_CONFIG
)
