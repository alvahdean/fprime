####
# library.cmake for ESP32
#
# Exposes ESP32-specific modules, subtopologies, and platform configuration.
####

add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/config/ESP32FreeRTOS")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/ESP32/Drv/Esp32UartDriver")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/ESP32/Drv/Esp32WifiDriver")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/ESP32/Svc/FreeRtosTimer")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/ESP32/Subtopologies/ESP32CdhCore")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/ESP32/Subtopologies/ESP32ComCcsds")
