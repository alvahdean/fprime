restrict_platforms(ESP32FreeRTOS)

register_fprime_module(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/FreeRtosTimer.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/FreeRtosTimer.cpp"
)
