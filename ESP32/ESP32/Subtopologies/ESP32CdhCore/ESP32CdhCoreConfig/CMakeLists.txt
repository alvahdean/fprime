register_fprime_config(
    EXCLUDE_FROM_ALL
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/ESP32CdhCoreConfig.fpp"
        "${CMAKE_CURRENT_LIST_DIR}/ESP32CdhCoreFatalHandlerConfig.fpp"
        "${CMAKE_CURRENT_LIST_DIR}/ESP32CdhCoreTlmConfig.fpp"
    INTERFACE
)
