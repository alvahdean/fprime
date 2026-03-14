
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/ESP32ComCcsdsConfig/")

register_fprime_module(
    EXCLUDE_FROM_ALL
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/ESP32ComCcsds.fpp"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/SubtopologyTopologyDefs.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/PingEntries.hpp"
    DEPENDS
        ESP32_Subtopologies_ESP32ComCcsds_ESP32ComCcsdsConfig
    INTERFACE
)
