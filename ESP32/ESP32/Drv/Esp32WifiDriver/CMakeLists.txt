restrict_platforms(ESP32FreeRTOS)

register_fprime_module(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/Esp32WifiDriver.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/Esp32WifiDriver.cpp"
    DEPENDS
        Fw_Buffer
)
