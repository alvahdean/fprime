// ======================================================================
// \title  FreeRTOS/Os/RawTime.hpp
// \brief  FreeRTOS implementation for Os::RawTime
// ======================================================================
#ifndef OS_FREERTOS_RAWTIME_HPP
#define OS_FREERTOS_RAWTIME_HPP

#include <Os/RawTime.hpp>

namespace Os {
namespace FreeRTOS {
namespace RawTime {

struct FreeRtosRawTimeHandle : public RawTimeHandle {
    U64 m_microseconds = 0;
};

class FreeRtosRawTime final : public RawTimeInterface {
  public:
    FreeRtosRawTime() = default;
    FreeRtosRawTime(const FreeRtosRawTime& other) = default;
    FreeRtosRawTime& operator=(const FreeRtosRawTime& other) = default;
    ~FreeRtosRawTime() override = default;

    RawTimeHandle* getHandle() override;

    Status now() override;
    Status getTimeInterval(const Os::RawTime& other, Fw::TimeInterval& interval) const override;
    Fw::SerializeStatus serializeTo(Fw::SerialBufferBase& buffer,
                                    Fw::Endianness mode = Fw::Endianness::BIG) const override;
    Fw::SerializeStatus deserializeFrom(Fw::SerialBufferBase& buffer,
                                        Fw::Endianness mode = Fw::Endianness::BIG) override;

  private:
    FreeRtosRawTimeHandle m_handle;
};

}  // namespace RawTime
}  // namespace FreeRTOS
}  // namespace Os

#endif
