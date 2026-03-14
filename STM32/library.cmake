####
# library.cmake for STM32
#
# Exposes STM32-specific modules.
####

add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/STM32/Subtopologies/STM32CdhCore")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/STM32/Subtopologies/STM32ComFprime")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/STM32/Drv/STM32UartDriver")
