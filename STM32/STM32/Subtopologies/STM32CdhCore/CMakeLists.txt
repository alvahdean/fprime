add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/STM32CdhCoreConfig/")

register_fprime_module(
    EXCLUDE_FROM_ALL
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/STM32CdhCore.fpp"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/SubtopologyTopologyDefs.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/PingEntries.hpp"
    INTERFACE
    DEPENDS
        STM32_Subtopologies_STM32CdhCore_STM32CdhCoreConfig
)
