// ======================================================================
// \title  FreeRTOS/Os/Memory.cpp
// \brief  FreeRTOS implementation for Os::Memory
// ======================================================================

#include "FreeRTOS/Os/Memory.hpp"

#include <limits>

#include "FreeRTOS/Os/FreeRTOSApi.hpp"

namespace Os {
namespace FreeRTOS {
namespace Memory {

MemoryInterface::Status FreeRtosMemory::_getUsage(Usage& memory_usage) {
#if defined(configTOTAL_HEAP_SIZE)
    const FwSizeType total = static_cast<FwSizeType>(configTOTAL_HEAP_SIZE);
    const FwSizeType free = static_cast<FwSizeType>(xPortGetFreeHeapSize());

    if (free > total) {
        memory_usage.total = 1;
        memory_usage.used = 1;
        return Status::ERROR;
    }

    memory_usage.total = total;
    memory_usage.used = total - free;
    return Status::OP_OK;
#else
    memory_usage.total = 0;
    memory_usage.used = 0;
    return Status::ERROR;
#endif
}

MemoryHandle* FreeRtosMemory::getHandle() {
    return &this->m_handle;
}

}  // namespace Memory
}  // namespace FreeRTOS
}  // namespace Os
