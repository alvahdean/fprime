// ======================================================================
// \title  FreeRTOS/Os/Task.hpp
// \brief  FreeRTOS implementation for Os::Task
// ======================================================================
#ifndef OS_FREERTOS_TASK_HPP
#define OS_FREERTOS_TASK_HPP

#include <Os/Task.hpp>

#include "FreeRTOS/Os/FreeRTOSApi.hpp"

namespace Os {
namespace FreeRTOS {
namespace Task {

struct FreeRtosTaskHandle : public TaskHandle {
    TaskHandle_t m_task = nullptr;
    bool m_is_valid = false;
};

class FreeRtosTask final : public TaskInterface {
  public:
    FreeRtosTask() = default;
    ~FreeRtosTask() override;

    void onStart() override;
    Status start(const Arguments& arguments) override;
    Status join() override;
    void suspend(SuspensionType suspension_type) override;
    void resume() override;
    Status _delay(const Fw::TimeInterval& interval) override;
    TaskHandle* getHandle() override;

  private:
    struct TaskState {
        SemaphoreHandle_t m_join_semaphore = nullptr;
    };

    struct TaskEntryContext {
        taskRoutine m_routine = nullptr;
        void* m_argument = nullptr;
        SemaphoreHandle_t m_join_semaphore = nullptr;
    };

    static void taskEntry(void* context_pointer);

    void cleanupState();

    FreeRtosTaskHandle m_handle;
    TaskState* m_state = nullptr;
};

}  // namespace Task
}  // namespace FreeRTOS
}  // namespace Os

#endif
