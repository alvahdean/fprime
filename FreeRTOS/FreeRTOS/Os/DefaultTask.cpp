// ======================================================================
// \title  FreeRTOS/Os/DefaultTask.cpp
// \brief  Sets default Os::Task implementation to FreeRTOS
// ======================================================================

#include "Os/Delegate.hpp"
#include "FreeRTOS/Os/Task.hpp"
#include "Os/Task.hpp"

namespace Os {

TaskInterface* TaskInterface::getDelegate(TaskHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<TaskInterface, Os::FreeRTOS::Task::FreeRtosTask>(aligned_new_memory);
}

}  // namespace Os
