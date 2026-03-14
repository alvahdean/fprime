restrict_platforms(ESP32FreeRTOS)

register_fprime_module(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/Esp32UartDriver.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/Esp32UartDriver.cpp"
    DEPENDS
        Fw_Buffer
)
