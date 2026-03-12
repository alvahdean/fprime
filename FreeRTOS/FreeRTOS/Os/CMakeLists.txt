####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
#
####
restrict_platforms(FreeRTOS)
add_custom_target("${FPRIME_CURRENT_MODULE}")

register_fprime_module(
    Os_FreeRTOS_Support
  SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/FreeRTOSSupport.cpp"
  HEADERS
    "${CMAKE_CURRENT_LIST_DIR}/FreeRTOSApi.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/FreeRTOSSupport.hpp"
  DEPENDS
    Fw_Time
    Fw_Types
)

register_os_implementation("File;FileSystem;Directory" FreeRTOS Os_FreeRTOS_Support)
register_os_implementation(Console FreeRTOS Os_FreeRTOS_Support)
register_os_implementation(Task FreeRTOS Os_FreeRTOS_Support)
register_os_implementation("Mutex;ConditionVariable" FreeRTOS Os_FreeRTOS_Support)
register_os_implementation(Queue FreeRTOS Os_FreeRTOS_Support Os_Generic_Types)
register_os_implementation(RawTime FreeRTOS Os_FreeRTOS_Support)
register_os_implementation(Cpu FreeRTOS Os_FreeRTOS_Support)
register_os_implementation(Memory FreeRTOS Os_FreeRTOS_Support)

if (BUILD_TESTING)
    add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/test")
endif()
