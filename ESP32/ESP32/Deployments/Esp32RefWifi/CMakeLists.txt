cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.26)
project(Esp32RefWifi VERSION 0.1.0 LANGUAGES C CXX)

set(FPRIME_INCLUDE_FRAMEWORK_CODE ON)
find_package(FPrime REQUIRED PATHS "${CMAKE_CURRENT_LIST_DIR}/../../../..")
include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/esp32_idf_wrapper.cmake")

add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../Svc/LedController")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../Drv/Esp32GpioDriver")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/config")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Top")

register_fprime_deployment(
    Esp32RefWifi
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/Main.cpp"
    DEPENDS
        ESP32_Svc_LedController
        ESP32_Drv_Esp32GpioDriver
        ESP32_Deployments_Esp32RefWifi_Top
        ESP32_Drv_Esp32WifiDriver
        ESP32_Subtopologies_ESP32CdhCore
        ESP32_Subtopologies_ESP32ComCcsds
        Svc_OsTime
)

add_custom_target(ESP32_Deployments_Esp32RefWifi DEPENDS Esp32RefWifi)
esp32_register_flash_image_target(Esp32RefWifi)
