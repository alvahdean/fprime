// ======================================================================
// \title  FreeRTOS/Os/RawTime.cpp
// \brief  FreeRTOS implementation for Os::RawTime
// ======================================================================

#include "FreeRTOS/Os/RawTime.hpp"

#include <atomic>
#include <limits>

#include "FreeRTOS/Os/FreeRTOSApi.hpp"
#include "FreeRTOS/Os/FreeRTOSSupport.hpp"

namespace Os {
namespace FreeRTOS {
namespace RawTime {

namespace {
std::atomic<TickType_t> s_last_tick{0};
std::atomic<U64> s_tick_wrap_offset{0};

U64 currentTickCountExtended() {
    const TickType_t current = xTaskGetTickCount();
    TickType_t previous = s_last_tick.load(std::memory_order_relaxed);
    while (!s_last_tick.compare_exchange_weak(previous, current, std::memory_order_acq_rel, std::memory_order_relaxed)) {
    }

    if (current < previous) {
        const U64 wrap_amount = static_cast<U64>(std::numeric_limits<TickType_t>::max()) + 1ULL;
        s_tick_wrap_offset.fetch_add(wrap_amount, std::memory_order_acq_rel);
    }

    return s_tick_wrap_offset.load(std::memory_order_acquire) + static_cast<U64>(current);
}

U64 ticksToMicroseconds(U64 ticks) {
    return (ticks * 1000000ULL) / static_cast<U64>(configTICK_RATE_HZ);
}

}  // namespace

RawTimeHandle* FreeRtosRawTime::getHandle() {
    return &this->m_handle;
}

FreeRtosRawTime::Status FreeRtosRawTime::now() {
    const U64 extended_ticks = currentTickCountExtended();
    U64 monotonic_usec = ticksToMicroseconds(extended_ticks);

    const Os::FreeRTOSSupport::RawTimeEpochHook hook = Os::FreeRTOSSupport::getRawTimeEpochHook();
    if (hook != nullptr) {
        const U64 mono_sec_u64 = monotonic_usec / 1000000ULL;
        const U64 mono_usec_u64 = monotonic_usec % 1000000ULL;
        if ((mono_sec_u64 <= std::numeric_limits<U32>::max()) && (mono_usec_u64 <= std::numeric_limits<U32>::max())) {
            U32 epoch_seconds = 0;
            U32 epoch_useconds = 0;
            if (hook(static_cast<U32>(mono_sec_u64), static_cast<U32>(mono_usec_u64), epoch_seconds, epoch_useconds)) {
                monotonic_usec = static_cast<U64>(epoch_seconds) * 1000000ULL + static_cast<U64>(epoch_useconds);
            }
        }
    }

    this->m_handle.m_microseconds = monotonic_usec;
    return Status::OP_OK;
}

FreeRtosRawTime::Status FreeRtosRawTime::getTimeInterval(const Os::RawTime& other, Fw::TimeInterval& interval) const {
    const FreeRtosRawTimeHandle* other_handle =
        static_cast<FreeRtosRawTimeHandle*>(const_cast<Os::RawTime&>(other).getHandle());

    U64 newer = this->m_handle.m_microseconds;
    U64 older = other_handle->m_microseconds;
    if (newer < older) {
        const U64 tmp = newer;
        newer = older;
        older = tmp;
    }

    const U64 delta = newer - older;
    const U64 seconds = delta / 1000000ULL;
    const U64 useconds = delta % 1000000ULL;
    if (seconds > std::numeric_limits<U32>::max()) {
        return Status::OP_OVERFLOW;
    }

    interval.set(static_cast<U32>(seconds), static_cast<U32>(useconds));
    return Status::OP_OK;
}

Fw::SerializeStatus FreeRtosRawTime::serializeTo(Fw::SerialBufferBase& buffer, Fw::Endianness mode) const {
    static_assert(FreeRtosRawTime::SERIALIZED_SIZE >= (2 * sizeof(U32)),
                  "FreeRTOS RawTime needs at least 8 bytes of serialization capacity");

    const U64 seconds_u64 = this->m_handle.m_microseconds / 1000000ULL;
    const U64 nanoseconds_u64 = (this->m_handle.m_microseconds % 1000000ULL) * 1000ULL;
    if ((seconds_u64 > std::numeric_limits<U32>::max()) || (nanoseconds_u64 > std::numeric_limits<U32>::max())) {
        return Fw::SerializeStatus::FW_SERIALIZE_FORMAT_ERROR;
    }

    Fw::SerializeStatus status = buffer.serializeFrom(static_cast<U32>(seconds_u64), mode);
    if (status != Fw::SerializeStatus::FW_SERIALIZE_OK) {
        return status;
    }
    return buffer.serializeFrom(static_cast<U32>(nanoseconds_u64), mode);
}

Fw::SerializeStatus FreeRtosRawTime::deserializeFrom(Fw::SerialBufferBase& buffer, Fw::Endianness mode) {
    static_assert(FreeRtosRawTime::SERIALIZED_SIZE >= (2 * sizeof(U32)),
                  "FreeRTOS RawTime needs at least 8 bytes of serialization capacity");

    U32 seconds = 0;
    U32 nanoseconds = 0;

    Fw::SerializeStatus status = buffer.deserializeTo(seconds, mode);
    if (status != Fw::SerializeStatus::FW_SERIALIZE_OK) {
        return status;
    }

    status = buffer.deserializeTo(nanoseconds, mode);
    if (status != Fw::SerializeStatus::FW_SERIALIZE_OK) {
        return status;
    }

    this->m_handle.m_microseconds = static_cast<U64>(seconds) * 1000000ULL + static_cast<U64>(nanoseconds) / 1000ULL;
    return Fw::SerializeStatus::FW_SERIALIZE_OK;
}

}  // namespace RawTime
}  // namespace FreeRTOS
}  // namespace Os
