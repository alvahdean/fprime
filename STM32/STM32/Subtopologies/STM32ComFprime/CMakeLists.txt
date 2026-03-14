add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/STM32ComFprimeConfig/")

register_fprime_module(
    EXCLUDE_FROM_ALL
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/STM32ComFprime.fpp"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/SubtopologyTopologyDefs.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/PingEntries.hpp"
    DEPENDS
        STM32_Subtopologies_STM32ComFprime_STM32ComFprimeConfig
    INTERFACE
)
