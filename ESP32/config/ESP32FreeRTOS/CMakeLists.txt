restrict_platforms(ESP32FreeRTOS)

add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Platform")

register_fprime_config(
        ESP32FreeRTOSConfig
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/ESP32FreeRTOSCfg.fpp"
    INTERFACE
)
