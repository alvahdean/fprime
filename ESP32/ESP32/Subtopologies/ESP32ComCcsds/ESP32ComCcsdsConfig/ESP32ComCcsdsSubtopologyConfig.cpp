#include "ESP32ComCcsdsSubtopologyConfig.hpp"

namespace ESP32ComCcsds {
namespace Allocation {
Fw::MallocAllocator mallocatorInstance;
Fw::MemAllocator& memAllocator = mallocatorInstance;
}  // namespace Allocation
}  // namespace ESP32ComCcsds
