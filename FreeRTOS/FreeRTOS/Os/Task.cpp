// ======================================================================
// \title  FreeRTOS/Os/Task.cpp
// \brief  FreeRTOS implementation for Os::Task
// ======================================================================

#include "FreeRTOS/Os/Task.hpp"

#include <Fw/Types/Assert.hpp>
#include <new>

#include "FreeRTOS/Os/FreeRTOSSupport.hpp"

namespace Os {
namespace FreeRTOS {
namespace Task {

namespace {

UBaseType_t convertPriority(const FwTaskPriorityType priority, Os::Task::Status& status) {
    status = Os::Task::Status::OP_OK;
    if (priority == Os::Task::TASK_PRIORITY_DEFAULT) {
        return static_cast<UBaseType_t>(tskIDLE_PRIORITY + 1);
    }

#if defined(configMAX_PRIORITIES)
    if (priority >= static_cast<FwTaskPriorityType>(configMAX_PRIORITIES)) {
        status = Os::Task::Status::INVALID_PRIORITY;
        return 0;
    }
#endif

    return static_cast<UBaseType_t>(priority);
}

UBaseType_t convertStackWords(const FwSizeType stack_size, Os::Task::Status& status) {
    status = Os::Task::Status::OP_OK;
    if (stack_size == Os::Task::TASK_DEFAULT) {
        return static_cast<UBaseType_t>(configMINIMAL_STACK_SIZE);
    }

    if (stack_size == 0) {
        status = Os::Task::Status::INVALID_STACK;
        return 0;
    }

    const FwSizeType bytes_per_word = sizeof(StackType_t);
    UBaseType_t words = static_cast<UBaseType_t>((stack_size + (bytes_per_word - 1)) / bytes_per_word);
    if (words == 0) {
        words = 1;
    }
    return words;
}

TickType_t intervalToTicks(const Fw::TimeInterval& interval, Os::Task::Status& status) {
    status = Os::Task::Status::OP_OK;
    const U64 usec = static_cast<U64>(interval.getSeconds()) * 1000000ULL + interval.getUSeconds();
    if (usec == 0ULL) {
        return 0;
    }
    const U64 ticks = (usec * static_cast<U64>(configTICK_RATE_HZ) + 999999ULL) / 1000000ULL;
    if (ticks == 0ULL) {
        return 1;
    }
    if (ticks > static_cast<U64>(portMAX_DELAY)) {
        return portMAX_DELAY;
    }
    return static_cast<TickType_t>(ticks);
}

}  // namespace

FreeRtosTask::~FreeRtosTask() {
    this->cleanupState();
}

void FreeRtosTask::onStart() {}

void FreeRtosTask::cleanupState() {
    if (this->m_state != nullptr) {
        if (this->m_state->m_join_semaphore != nullptr) {
            vSemaphoreDelete(this->m_state->m_join_semaphore);
            this->m_state->m_join_semaphore = nullptr;
        }
        delete this->m_state;
        this->m_state = nullptr;
    }
    this->m_handle.m_task = nullptr;
    this->m_handle.m_is_valid = false;
}

void FreeRtosTask::taskEntry(void* context_pointer) {
    TaskEntryContext* context = reinterpret_cast<TaskEntryContext*>(context_pointer);
    FW_ASSERT(context != nullptr);
    FW_ASSERT(context->m_routine != nullptr);

    context->m_routine(context->m_argument);

    if (context->m_join_semaphore != nullptr) {
        (void)xSemaphoreGive(context->m_join_semaphore);
    }

    delete context;
    vTaskDelete(nullptr);
}

Os::Task::Status FreeRtosTask::start(const Arguments& arguments) {
    if (arguments.m_routine == nullptr) {
        return Os::Task::Status::INVALID_PARAMS;
    }
    if (this->m_handle.m_is_valid) {
        return Os::Task::Status::INVALID_STATE;
    }

    this->cleanupState();

    TaskState* state = new (std::nothrow) TaskState();
    if (state == nullptr) {
        return Os::Task::Status::ERROR_RESOURCES;
    }

    state->m_join_semaphore = xSemaphoreCreateBinary();
    if (state->m_join_semaphore == nullptr) {
        delete state;
        return Os::Task::Status::ERROR_RESOURCES;
    }

    TaskEntryContext* context = new (std::nothrow) TaskEntryContext();
    if (context == nullptr) {
        vSemaphoreDelete(state->m_join_semaphore);
        delete state;
        return Os::Task::Status::ERROR_RESOURCES;
    }

    context->m_routine = arguments.m_routine;
    context->m_argument = arguments.m_routine_argument;
    context->m_join_semaphore = state->m_join_semaphore;

    Os::Task::Status priority_status = Os::Task::Status::OP_OK;
    const UBaseType_t priority = convertPriority(arguments.m_priority, priority_status);
    if (priority_status != Os::Task::Status::OP_OK) {
        delete context;
        vSemaphoreDelete(state->m_join_semaphore);
        delete state;
        return priority_status;
    }

    Os::Task::Status stack_status = Os::Task::Status::OP_OK;
    const UBaseType_t stack_words = convertStackWords(arguments.m_stackSize, stack_status);
    if (stack_status != Os::Task::Status::OP_OK) {
        delete context;
        vSemaphoreDelete(state->m_join_semaphore);
        delete state;
        return stack_status;
    }

    TaskHandle_t task_handle = nullptr;

#if defined(configSUPPORT_DYNAMIC_ALLOCATION) && (configSUPPORT_DYNAMIC_ALLOCATION == 1)
    BaseType_t create_status = xTaskCreate(taskEntry,
                                           arguments.m_name.toChar(),
                                           stack_words,
                                           context,
                                           priority,
                                           &task_handle);
    if (create_status != pdPASS) {
        task_handle = nullptr;
    }
#endif

#if defined(configSUPPORT_STATIC_ALLOCATION) && (configSUPPORT_STATIC_ALLOCATION == 1)
    if (task_handle == nullptr) {
        Os::FreeRTOSSupport::StaticTaskAllocator allocator = Os::FreeRTOSSupport::getStaticTaskAllocator();
        if (allocator != nullptr) {
            StaticTask_t* task_buffer = nullptr;
            StackType_t* stack_buffer = nullptr;
            U32 stack_depth_words = static_cast<U32>(stack_words);
            if (allocator(arguments, task_buffer, stack_buffer, stack_depth_words)) {
                task_handle = xTaskCreateStatic(taskEntry,
                                                arguments.m_name.toChar(),
                                                static_cast<UBaseType_t>(stack_depth_words),
                                                context,
                                                priority,
                                                stack_buffer,
                                                task_buffer);
            }
        }
    }
#endif

    if (task_handle == nullptr) {
        delete context;
        vSemaphoreDelete(state->m_join_semaphore);
        delete state;
        return Os::Task::Status::ERROR_RESOURCES;
    }

    this->m_state = state;
    this->m_handle.m_task = task_handle;
    this->m_handle.m_is_valid = true;
    return Os::Task::Status::OP_OK;
}

Os::Task::Status FreeRtosTask::join() {
    if ((!this->m_handle.m_is_valid) || (this->m_state == nullptr) || (this->m_state->m_join_semaphore == nullptr)) {
        return Os::Task::Status::INVALID_HANDLE;
    }

    if (xSemaphoreTake(this->m_state->m_join_semaphore, portMAX_DELAY) != pdTRUE) {
        return Os::Task::Status::JOIN_ERROR;
    }

    this->cleanupState();
    return Os::Task::Status::OP_OK;
}

void FreeRtosTask::suspend(SuspensionType suspension_type) {
    (void)suspension_type;
    if (this->m_handle.m_is_valid && this->m_handle.m_task != nullptr) {
        vTaskSuspend(this->m_handle.m_task);
    }
}

void FreeRtosTask::resume() {
    if (this->m_handle.m_is_valid && this->m_handle.m_task != nullptr) {
        vTaskResume(this->m_handle.m_task);
    }
}

Os::Task::Status FreeRtosTask::_delay(const Fw::TimeInterval& interval) {
    Os::Task::Status status = Os::Task::Status::OP_OK;
    const TickType_t ticks = intervalToTicks(interval, status);
    if (status != Os::Task::Status::OP_OK) {
        return status;
    }
    if (ticks > 0) {
        vTaskDelay(ticks);
    }
    return Os::Task::Status::OP_OK;
}

TaskHandle* FreeRtosTask::getHandle() {
    return &this->m_handle;
}

}  // namespace Task
}  // namespace FreeRTOS
}  // namespace Os
