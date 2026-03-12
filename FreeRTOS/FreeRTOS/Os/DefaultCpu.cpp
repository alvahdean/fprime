// ======================================================================
// \title  FreeRTOS/Os/DefaultCpu.cpp
// \brief  Sets default Os::Cpu implementation to FreeRTOS
// ======================================================================

#include "Os/Cpu.hpp"
#include "Os/Delegate.hpp"
#include "FreeRTOS/Os/Cpu.hpp"

namespace Os {

CpuInterface* CpuInterface::getDelegate(CpuHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<CpuInterface, Os::FreeRTOS::Cpu::FreeRtosCpu>(aligned_new_memory);
}

}  // namespace Os
