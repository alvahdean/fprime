// ======================================================================
// \title  FreeRTOS/Os/DefaultRawTime.cpp
// \brief  Sets default Os::RawTime implementation to FreeRTOS
// ======================================================================

#include "Os/Delegate.hpp"
#include "FreeRTOS/Os/RawTime.hpp"

namespace Os {

RawTimeInterface* RawTimeInterface::getDelegate(RawTimeHandleStorage& aligned_new_memory,
                                                const RawTimeInterface* to_copy) {
    return Os::Delegate::makeDelegate<RawTimeInterface, Os::FreeRTOS::RawTime::FreeRtosRawTime, RawTimeHandleStorage>(
        aligned_new_memory, to_copy);
}

}  // namespace Os
