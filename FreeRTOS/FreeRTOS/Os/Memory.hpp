// ======================================================================
// \title  FreeRTOS/Os/Memory.hpp
// \brief  FreeRTOS implementation for Os::Memory
// ======================================================================
#ifndef OS_FREERTOS_MEMORY_HPP
#define OS_FREERTOS_MEMORY_HPP

#include <Os/Memory.hpp>

namespace Os {
namespace FreeRTOS {
namespace Memory {

struct FreeRtosMemoryHandle : public MemoryHandle {};

class FreeRtosMemory final : public MemoryInterface {
  public:
    using MemoryInterface::operator=;
    FreeRtosMemory() = default;
    FreeRtosMemory(const FreeRtosMemory&) = delete;
    FreeRtosMemory& operator=(const FreeRtosMemory&) = delete;
    ~FreeRtosMemory() override = default;

    Status _getUsage(Usage& memory_usage) override;
    MemoryHandle* getHandle() override;

  private:
    FreeRtosMemoryHandle m_handle;
};

}  // namespace Memory
}  // namespace FreeRTOS
}  // namespace Os

#endif
