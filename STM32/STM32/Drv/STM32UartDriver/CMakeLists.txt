####
# F prime CMakeLists.txt:
#
# STM32 UART byte stream driver skeleton (FreeRTOS platform only).
# Host-side simulation UTs are platform-agnostic and build when BUILD_TESTING=ON.
####

if (FPRIME_PLATFORM STREQUAL "FreeRtosStm32" OR BUILD_TESTING)
    set(MOD_DEPS Os)
    set(SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/STM32UartDriver.fpp"
        "${CMAKE_CURRENT_LIST_DIR}/STM32UartDriver.cpp"
    )
    register_fprime_module()
endif()

if (BUILD_TESTING)
    set(UT_SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/STM32UartDriver.fpp"
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/STM32UartDriverTestMain.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/STM32UartDriverTester.cpp"
    )
    set(UT_MOD_DEPS
        STM32_Drv_STM32UartDriver
        STest
    )
    set(UT_AUTO_HELPERS ON)
    register_fprime_ut("Drv_STM32UartDriver_SimUT")
endif()
