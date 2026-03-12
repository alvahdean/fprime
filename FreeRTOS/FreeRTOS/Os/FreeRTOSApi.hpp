// ======================================================================
// \title  FreeRTOS/Os/FreeRTOSApi.hpp
// \brief  Compatibility include for FreeRTOS kernel APIs
// ======================================================================
#ifndef OS_FREERTOS_API_HPP
#define OS_FREERTOS_API_HPP

#include <Fw/FPrimeBasicTypes.hpp>

#if defined(__has_include)
#if __has_include("FreeRTOS.h")
#define OS_FREERTOS_HAS_KERNEL 1
extern "C" {
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
}
#else
#define OS_FREERTOS_HAS_KERNEL 0
#endif
#else
#define OS_FREERTOS_HAS_KERNEL 0
#endif

#if !OS_FREERTOS_HAS_KERNEL

#include <cstddef>
#include <cstdint>

using BaseType_t = int;
using UBaseType_t = unsigned int;
using TickType_t = std::uint32_t;
using StackType_t = std::uint32_t;

struct StaticTask_t {
    U32 m_data[4];
};

struct StaticSemaphore_t {
    U32 m_data[4];
};

struct OpaqueTask;
struct OpaqueSemaphore;

using TaskHandle_t = OpaqueTask*;
using SemaphoreHandle_t = OpaqueSemaphore*;

#ifndef pdTRUE
#define pdTRUE 1
#endif
#ifndef pdFALSE
#define pdFALSE 0
#endif
#ifndef pdPASS
#define pdPASS 1
#endif
#ifndef pdFAIL
#define pdFAIL 0
#endif
#ifndef tskIDLE_PRIORITY
#define tskIDLE_PRIORITY 0
#endif
#ifndef portMAX_DELAY
#define portMAX_DELAY static_cast<TickType_t>(0xFFFFFFFFu)
#endif
#ifndef configTICK_RATE_HZ
#define configTICK_RATE_HZ 1000
#endif
#ifndef configMINIMAL_STACK_SIZE
#define configMINIMAL_STACK_SIZE 256
#endif
#ifndef configMAX_PRIORITIES
#define configMAX_PRIORITIES 32
#endif
#ifndef configNUM_CORES
#define configNUM_CORES 1
#endif
#ifndef configSUPPORT_DYNAMIC_ALLOCATION
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#endif
#ifndef configSUPPORT_STATIC_ALLOCATION
#define configSUPPORT_STATIC_ALLOCATION 0
#endif

inline BaseType_t xTaskCreate(void (*taskFunction)(void*),
                              const char* name,
                              UBaseType_t stackDepth,
                              void* parameters,
                              UBaseType_t priority,
                              TaskHandle_t* taskHandle) {
    (void)taskFunction;
    (void)name;
    (void)stackDepth;
    (void)parameters;
    (void)priority;
    if (taskHandle != nullptr) {
        *taskHandle = nullptr;
    }
    return pdFAIL;
}

inline TaskHandle_t xTaskCreateStatic(void (*taskFunction)(void*),
                                      const char* name,
                                      UBaseType_t stackDepth,
                                      void* parameters,
                                      UBaseType_t priority,
                                      StackType_t* stackBuffer,
                                      StaticTask_t* taskBuffer) {
    (void)taskFunction;
    (void)name;
    (void)stackDepth;
    (void)parameters;
    (void)priority;
    (void)stackBuffer;
    (void)taskBuffer;
    return nullptr;
}

inline void vTaskDelete(TaskHandle_t taskHandle) {
    (void)taskHandle;
}

inline void vTaskSuspend(TaskHandle_t taskHandle) {
    (void)taskHandle;
}

inline void vTaskResume(TaskHandle_t taskHandle) {
    (void)taskHandle;
}

inline void vTaskDelay(TickType_t ticksToDelay) {
    (void)ticksToDelay;
}

inline TaskHandle_t xTaskGetCurrentTaskHandle() {
    return nullptr;
}

inline TickType_t xTaskGetTickCount() {
    return 0;
}

inline SemaphoreHandle_t xSemaphoreCreateMutex() {
    return nullptr;
}

inline SemaphoreHandle_t xSemaphoreCreateBinary() {
    return nullptr;
}

inline SemaphoreHandle_t xSemaphoreCreateCounting(UBaseType_t maxCount, UBaseType_t initialCount) {
    (void)maxCount;
    (void)initialCount;
    return nullptr;
}

inline BaseType_t xSemaphoreTake(SemaphoreHandle_t semaphore, TickType_t ticksToWait) {
    (void)semaphore;
    (void)ticksToWait;
    return pdFALSE;
}

inline BaseType_t xSemaphoreGive(SemaphoreHandle_t semaphore) {
    (void)semaphore;
    return pdFALSE;
}

inline void vSemaphoreDelete(SemaphoreHandle_t semaphore) {
    (void)semaphore;
}

inline void taskENTER_CRITICAL() {}
inline void taskEXIT_CRITICAL() {}

inline std::size_t xPortGetFreeHeapSize() {
    return 0;
}

inline std::size_t xPortGetMinimumEverFreeHeapSize() {
    return 0;
}

#endif

#endif
