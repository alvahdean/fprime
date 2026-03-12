// ======================================================================
// \title  FreeRTOS/Os/Mutex.hpp
// \brief  FreeRTOS implementation for Os::Mutex
// ======================================================================
#ifndef OS_FREERTOS_MUTEX_HPP
#define OS_FREERTOS_MUTEX_HPP

#include <Os/Mutex.hpp>

#include "FreeRTOS/Os/FreeRTOSApi.hpp"

namespace Os {
namespace FreeRTOS {
namespace Mutex {

struct FreeRtosMutexHandle : public MutexHandle {
    SemaphoreHandle_t m_mutex = nullptr;
    TaskHandle_t m_owner = nullptr;
};

class FreeRtosMutex final : public MutexInterface {
  public:
    FreeRtosMutex();
    ~FreeRtosMutex() override;

    Status take() override;
    Status release() override;
    MutexHandle* getHandle() override;

  private:
    FreeRtosMutexHandle m_handle;
};

}  // namespace Mutex
}  // namespace FreeRTOS
}  // namespace Os

#endif
