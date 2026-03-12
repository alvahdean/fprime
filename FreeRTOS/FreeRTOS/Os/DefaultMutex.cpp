// ======================================================================
// \title  FreeRTOS/Os/DefaultMutex.cpp
// \brief  Sets default Os::Mutex/ConditionVariable implementation to FreeRTOS
// ======================================================================

#include "Os/Delegate.hpp"
#include "FreeRTOS/Os/ConditionVariable.hpp"
#include "FreeRTOS/Os/Mutex.hpp"

namespace Os {

MutexInterface* MutexInterface::getDelegate(MutexHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<MutexInterface, Os::FreeRTOS::Mutex::FreeRtosMutex>(aligned_new_memory);
}

ConditionVariableInterface* ConditionVariableInterface::getDelegate(ConditionVariableHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<ConditionVariableInterface, Os::FreeRTOS::Mutex::FreeRtosConditionVariable,
                                      ConditionVariableHandleStorage>(aligned_new_memory);
}

}  // namespace Os
