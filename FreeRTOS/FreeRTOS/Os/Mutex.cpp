// ======================================================================
// \title  FreeRTOS/Os/Mutex.cpp
// \brief  FreeRTOS implementation for Os::Mutex
// ======================================================================

#include "FreeRTOS/Os/Mutex.hpp"

#include "Fw/Types/Assert.hpp"

namespace Os {
namespace FreeRTOS {
namespace Mutex {

FreeRtosMutex::FreeRtosMutex() {
    this->m_handle.m_mutex = xSemaphoreCreateMutex();
}

FreeRtosMutex::~FreeRtosMutex() {
    if (this->m_handle.m_mutex != nullptr) {
        vSemaphoreDelete(this->m_handle.m_mutex);
        this->m_handle.m_mutex = nullptr;
        this->m_handle.m_owner = nullptr;
    }
}

FreeRtosMutex::Status FreeRtosMutex::take() {
    if (this->m_handle.m_mutex == nullptr) {
        return Status::NOT_SUPPORTED;
    }
    TaskHandle_t current = xTaskGetCurrentTaskHandle();
    if ((current != nullptr) && (this->m_handle.m_owner == current)) {
        return Status::ERROR_DEADLOCK;
    }
    if (xSemaphoreTake(this->m_handle.m_mutex, portMAX_DELAY) != pdTRUE) {
        return Status::ERROR_OTHER;
    }
    this->m_handle.m_owner = current;
    return Status::OP_OK;
}

FreeRtosMutex::Status FreeRtosMutex::release() {
    if (this->m_handle.m_mutex == nullptr) {
        return Status::NOT_SUPPORTED;
    }
    TaskHandle_t current = xTaskGetCurrentTaskHandle();
    if ((this->m_handle.m_owner != nullptr) && (current != nullptr) && (this->m_handle.m_owner != current)) {
        return Status::ERROR_OTHER;
    }
    this->m_handle.m_owner = nullptr;
    if (xSemaphoreGive(this->m_handle.m_mutex) != pdTRUE) {
        return Status::ERROR_OTHER;
    }
    return Status::OP_OK;
}

MutexHandle* FreeRtosMutex::getHandle() {
    return &this->m_handle;
}

}  // namespace Mutex
}  // namespace FreeRTOS
}  // namespace Os
