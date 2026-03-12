####
# FreeRTOS OSAL unit tests
####
if (NOT BUILD_TESTING)
    return()
endif()

register_fprime_ut(
    FreeRTOSSupportTest
  SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/ut/FreeRTOSSupportTests.cpp"
  DEPENDS
    Fw_Types
    Os
    STest
  CHOOSES_IMPLEMENTATIONS
    Os_File_FreeRTOS
    Os_Console_FreeRTOS
)

option(OS_FREERTOS_ENABLE_COMMON_UTS "Enable common Os rule-suite tests under FreeRTOS" OFF)
if (OS_FREERTOS_ENABLE_COMMON_UTS)
    register_fprime_ut(
        FreeRTOSTaskTest
      SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/../../test/ut/task/CommonTests.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/../../test/ut/task/TaskRules.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ut/FreeRTOSTaskTests.cpp"
      DEPENDS
        Fw_Types
        Os
        STest
      CHOOSES_IMPLEMENTATIONS
        Os_Task_FreeRTOS
    )

    register_fprime_ut(
        FreeRTOSMutexTest
      SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/../../test/ut/mutex/CommonTests.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/../../test/ut/mutex/MutexRules.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ut/FreeRTOSMutexTests.cpp"
      DEPENDS
        Fw_Types
        Os
        STest
      CHOOSES_IMPLEMENTATIONS
        Os_Mutex_FreeRTOS
    )

    register_fprime_ut(
        FreeRTOSConditionTest
      SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/../../test/ut/condition/CommonTests.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/../../test/ut/condition/ConditionRules.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ut/FreeRTOSConditionTests.cpp"
      DEPENDS
        Fw_Types
        Os
        STest
      CHOOSES_IMPLEMENTATIONS
        Os_Mutex_FreeRTOS
    )

    register_fprime_ut(
        FreeRTOSRawTimeTest
      SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/../../test/ut/rawtime/CommonTests.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/../../test/ut/rawtime/RawTimeRules.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ut/FreeRTOSRawTimeTests.cpp"
      DEPENDS
        Fw_Types
        Os
        STest
      CHOOSES_IMPLEMENTATIONS
        Os_RawTime_FreeRTOS
    )

    register_fprime_ut(
        FreeRTOSQueueTest
      SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/../../test/ut/queue/CommonTests.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/../../test/ut/queue/QueueRules.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ut/FreeRTOSQueueTests.cpp"
      DEPENDS
        Fw_Types
        Os
        STest
      CHOOSES_IMPLEMENTATIONS
        Os_Queue_FreeRTOS
    )
endif()
