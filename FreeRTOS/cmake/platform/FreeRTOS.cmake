####
# FreeRTOS.cmake:
#
# FreeRTOS platform file for FreeRTOS targets.
####

set(FPRIME_HAS_SOCKETS OFF)

# Reuse unix-compatible platform type aliases without unix Posix impl choices.
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/freertos/Platform/")

register_fprime_config(
    PlatformFreeRTOS
  INTERFACE
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

target_compile_definitions(PlatformFreeRTOS INTERFACE -DTGT_OS_TYPE_FREERTOS)
