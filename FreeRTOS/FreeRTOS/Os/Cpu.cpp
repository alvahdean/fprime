// ======================================================================
// \title  FreeRTOS/Os/Cpu.cpp
// \brief  FreeRTOS implementation for Os::Cpu
// ======================================================================

#include "FreeRTOS/Os/Cpu.hpp"

#include "FreeRTOS/Os/FreeRTOSApi.hpp"

namespace Os {
namespace FreeRTOS {
namespace Cpu {

CpuInterface::Status FreeRtosCpu::_getCount(FwSizeType& cpu_count) {
#if defined(configNUM_CORES)
    cpu_count = static_cast<FwSizeType>(configNUM_CORES);
#else
    cpu_count = 1;
#endif
    return Status::OP_OK;
}

CpuInterface::Status FreeRtosCpu::_getTicks(Ticks& ticks, FwSizeType cpu_index) {
    (void)cpu_index;
#if defined(configGENERATE_RUN_TIME_STATS) && (configGENERATE_RUN_TIME_STATS == 1) && defined(portGET_RUN_TIME_COUNTER_VALUE)
    const U32 total = static_cast<U32>(portGET_RUN_TIME_COUNTER_VALUE());
    ticks.total = total;
    ticks.used = total;
    return Status::OP_OK;
#else
    ticks.total = 1;
    ticks.used = 1;
    return Status::ERROR;
#endif
}

CpuHandle* FreeRtosCpu::getHandle() {
    return &this->m_handle;
}

}  // namespace Cpu
}  // namespace FreeRTOS
}  // namespace Os
