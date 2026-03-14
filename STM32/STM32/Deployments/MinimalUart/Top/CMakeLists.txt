####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: module dependencies
####

set(SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/instances.fpp"
    "${CMAKE_CURRENT_LIST_DIR}/topology.fpp"
    "${CMAKE_CURRENT_LIST_DIR}/STM32MinimalTopology.cpp"
)

set(MOD_DEPS
    Fw/Logger
    Svc/OsTime
    Svc/Subtopologies/CdhCore
    Svc/Subtopologies/ComFprime
    STM32/Drv/STM32UartDriver
)

register_fprime_module()
