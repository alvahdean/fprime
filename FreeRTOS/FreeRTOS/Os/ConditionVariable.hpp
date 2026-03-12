// ======================================================================
// \title  FreeRTOS/Os/ConditionVariable.hpp
// \brief  FreeRTOS implementation for Os::ConditionVariable
// ======================================================================
#ifndef OS_FREERTOS_CONDITIONVARIABLE_HPP
#define OS_FREERTOS_CONDITIONVARIABLE_HPP

#include <Os/Condition.hpp>

#include "FreeRTOS/Os/FreeRTOSApi.hpp"

namespace Os {
namespace FreeRTOS {
namespace Mutex {

struct FreeRtosConditionVariableHandle : public ConditionVariableHandle {
    SemaphoreHandle_t m_condition = nullptr;
    U32 m_waiters = 0;
    U32 m_pending_signals = 0;
};

class FreeRtosConditionVariable final : public ConditionVariableInterface {
  public:
    FreeRtosConditionVariable();
    ~FreeRtosConditionVariable() override;

    ConditionVariableInterface& operator=(const ConditionVariableInterface& other) override = delete;

    Status pend(Os::Mutex& mutex) override;
    void notify() override;
    void notifyAll() override;
    ConditionVariableHandle* getHandle() override;

  private:
    FreeRtosConditionVariableHandle m_handle;
};

}  // namespace Mutex
}  // namespace FreeRTOS
}  // namespace Os

#endif
