// ======================================================================
// \title  FreeRTOS/Os/ConditionVariable.cpp
// \brief  FreeRTOS implementation for Os::ConditionVariable
// ======================================================================

#include "FreeRTOS/Os/ConditionVariable.hpp"

#include "FreeRTOS/Os/Mutex.hpp"

namespace Os {
namespace FreeRTOS {
namespace Mutex {

namespace {
static constexpr UBaseType_t CONDITION_MAX_COUNT = 65535;
}

FreeRtosConditionVariable::FreeRtosConditionVariable() {
    this->m_handle.m_condition = xSemaphoreCreateCounting(CONDITION_MAX_COUNT, 0);
}

FreeRtosConditionVariable::~FreeRtosConditionVariable() {
    if (this->m_handle.m_condition != nullptr) {
        vSemaphoreDelete(this->m_handle.m_condition);
        this->m_handle.m_condition = nullptr;
    }
}

FreeRtosConditionVariable::Status FreeRtosConditionVariable::pend(Os::Mutex& mutex) {
    if (this->m_handle.m_condition == nullptr) {
        return Status::NOT_SUPPORTED;
    }

    taskENTER_CRITICAL();
    this->m_handle.m_waiters += 1;
    taskEXIT_CRITICAL();

    const Os::Mutex::Status release_status = mutex.release();
    if (release_status != Os::Mutex::Status::OP_OK) {
        taskENTER_CRITICAL();
        this->m_handle.m_waiters -= 1;
        taskEXIT_CRITICAL();
        return Status::ERROR_MUTEX_NOT_HELD;
    }

    bool observed_signal = false;
    while (!observed_signal) {
        if (xSemaphoreTake(this->m_handle.m_condition, portMAX_DELAY) != pdTRUE) {
            (void)mutex.take();
            return Status::ERROR_OTHER;
        }

        taskENTER_CRITICAL();
        if (this->m_handle.m_pending_signals > 0) {
            this->m_handle.m_pending_signals -= 1;
            this->m_handle.m_waiters -= 1;
            observed_signal = true;
        }
        taskEXIT_CRITICAL();
    }

    return (mutex.take() == Os::Mutex::Status::OP_OK) ? Status::OP_OK : Status::ERROR_OTHER;
}

void FreeRtosConditionVariable::notify() {
    if (this->m_handle.m_condition == nullptr) {
        return;
    }

    bool should_signal = false;
    taskENTER_CRITICAL();
    if (this->m_handle.m_waiters > this->m_handle.m_pending_signals) {
        this->m_handle.m_pending_signals += 1;
        should_signal = true;
    }
    taskEXIT_CRITICAL();

    if (should_signal) {
        (void)xSemaphoreGive(this->m_handle.m_condition);
    }
}

void FreeRtosConditionVariable::notifyAll() {
    if (this->m_handle.m_condition == nullptr) {
        return;
    }

    U32 to_signal = 0;
    taskENTER_CRITICAL();
    if (this->m_handle.m_waiters > this->m_handle.m_pending_signals) {
        to_signal = this->m_handle.m_waiters - this->m_handle.m_pending_signals;
        this->m_handle.m_pending_signals += to_signal;
    }
    taskEXIT_CRITICAL();

    for (U32 i = 0; i < to_signal; i++) {
        (void)xSemaphoreGive(this->m_handle.m_condition);
    }
}

ConditionVariableHandle* FreeRtosConditionVariable::getHandle() {
    return &this->m_handle;
}

}  // namespace Mutex
}  // namespace FreeRTOS
}  // namespace Os
