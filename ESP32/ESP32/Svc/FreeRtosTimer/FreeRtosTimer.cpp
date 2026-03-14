#include "ESP32/Svc/FreeRtosTimer/FreeRtosTimer.hpp"

#include <Os/Task.hpp>

namespace Svc {

FreeRtosTimer::FreeRtosTimer(const char* compName) : FreeRtosTimerComponentBase(compName), m_quit(false) {}

FreeRtosTimer::~FreeRtosTimer() {}

void FreeRtosTimer::startTimer(const Fw::TimeInterval& interval) {
    while (true) {
        Os::Task::delay(interval);
        this->m_mutex.lock();
        const bool quit = this->m_quit;
        this->m_mutex.unLock();
        if (quit) {
            return;
        }
        (void)this->m_rawTime.now();
        this->CycleOut_out(0, this->m_rawTime);
    }
}

void FreeRtosTimer::quit() {
    this->m_mutex.lock();
    this->m_quit = true;
    this->m_mutex.unLock();
}

}  // namespace Svc
