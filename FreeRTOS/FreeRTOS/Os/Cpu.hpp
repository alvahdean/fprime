// ======================================================================
// \title  FreeRTOS/Os/Cpu.hpp
// \brief  FreeRTOS implementation for Os::Cpu
// ======================================================================
#ifndef OS_FREERTOS_CPU_HPP
#define OS_FREERTOS_CPU_HPP

#include <Os/Cpu.hpp>

namespace Os {
namespace FreeRTOS {
namespace Cpu {

struct FreeRtosCpuHandle : public CpuHandle {};

class FreeRtosCpu final : public CpuInterface {
  public:
    using CpuInterface::operator=;
    FreeRtosCpu() = default;
    FreeRtosCpu(const FreeRtosCpu&) = delete;
    FreeRtosCpu& operator=(const FreeRtosCpu&) = delete;
    ~FreeRtosCpu() override = default;

    Status _getCount(FwSizeType& cpu_count) override;
    Status _getTicks(Ticks& ticks, FwSizeType cpu_index) override;
    CpuHandle* getHandle() override;

  private:
    FreeRtosCpuHandle m_handle;
};

}  // namespace Cpu
}  // namespace FreeRTOS
}  // namespace Os

#endif
