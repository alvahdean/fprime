// ======================================================================
// \title  FreeRTOS/Os/DefaultQueue.cpp
// \brief  Sets default Os::Queue implementation to FreeRTOS
// ======================================================================

#include "Os/Delegate.hpp"
#include "FreeRTOS/Os/Queue.hpp"
#include "Os/Queue.hpp"

namespace Os {

QueueInterface* QueueInterface::getDelegate(QueueHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<QueueInterface, Os::FreeRTOS::Queue::FreeRtosQueue, QueueHandleStorage>(
        aligned_new_memory);
}

}  // namespace Os
