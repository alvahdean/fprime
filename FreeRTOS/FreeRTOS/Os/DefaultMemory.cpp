// ======================================================================
// \title  FreeRTOS/Os/DefaultMemory.cpp
// \brief  Sets default Os::Memory implementation to FreeRTOS
// ======================================================================

#include "Os/Delegate.hpp"
#include "FreeRTOS/Os/Memory.hpp"
#include "Os/Memory.hpp"

namespace Os {

MemoryInterface* MemoryInterface::getDelegate(MemoryHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<MemoryInterface, Os::FreeRTOS::Memory::FreeRtosMemory>(aligned_new_memory);
}

}  // namespace Os
