#include "STM32ComFprimeSubtopologyConfig.hpp"

namespace STM32ComFprime {
namespace Allocation {
Fw::MallocAllocator mallocatorInstance;
Fw::MemAllocator& memAllocator = mallocatorInstance;
}  // namespace Allocation
}  // namespace STM32ComFprime
