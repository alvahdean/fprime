#ifndef FPRIME_ESP32_FREERTOS_TIMER_HPP
#define FPRIME_ESP32_FREERTOS_TIMER_HPP

#include <Fw/Time/TimeInterval.hpp>
#include <Os/Mutex.hpp>
#include <Os/RawTime.hpp>
#include "ESP32/Svc/FreeRtosTimer/FreeRtosTimerComponentAc.hpp"

namespace Svc {

class FreeRtosTimer final : public FreeRtosTimerComponentBase {
  public:
    explicit FreeRtosTimer(const char* compName);
    ~FreeRtosTimer() override;

    void startTimer(const Fw::TimeInterval& interval);
    void quit();

  private:
    Os::Mutex m_mutex;
    volatile bool m_quit;
    Os::RawTime m_rawTime;
};

}  // namespace Svc

#endif
