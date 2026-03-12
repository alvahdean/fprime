// ======================================================================
// \title  FreeRTOS/Os/Queue.hpp
// \brief  FreeRTOS hybrid priority queue implementation
// ======================================================================
#ifndef OS_FREERTOS_QUEUE_HPP
#define OS_FREERTOS_QUEUE_HPP

#include <Os/Queue.hpp>

#include "FreeRTOS/Os/FreeRTOSApi.hpp"
#include "Os/Generic/Types/MaxHeap.hpp"

namespace Os {
namespace FreeRTOS {
namespace Queue {

struct FreeRtosQueueHandle : public QueueHandle {
    SemaphoreHandle_t m_lock = nullptr;
    SemaphoreHandle_t m_slots = nullptr;
    SemaphoreHandle_t m_items = nullptr;

    Types::MaxHeap m_heap;
    U8* m_heap_pointer = nullptr;
    U8* m_data = nullptr;
    FwSizeType* m_indices = nullptr;
    FwSizeType* m_sizes = nullptr;

    FwSizeType m_depth = 0;
    FwSizeType m_max_size = 0;
    FwSizeType m_start_index = 0;
    FwSizeType m_stop_index = 0;
    FwSizeType m_high_water = 0;
    FwEnumStoreType m_id = 0;

    FwSizeType findIndex();
    void returnIndex(FwSizeType index);
    void storeData(FwSizeType index, const U8* source, FwSizeType size);
    void loadData(FwSizeType index, U8* destination, FwSizeType size) const;
};

class FreeRtosQueue final : public QueueInterface {
  public:
    using QueueInterface::operator=;
    FreeRtosQueue() = default;
    FreeRtosQueue(const FreeRtosQueue&) = delete;
    FreeRtosQueue& operator=(const FreeRtosQueue&) = delete;
    ~FreeRtosQueue() override;

    Status create(FwEnumStoreType id,
                  const Fw::ConstStringBase& name,
                  FwSizeType depth,
                  FwSizeType messageSize) override;
    void teardown() override;

    Status send(const U8* buffer, FwSizeType size, FwQueuePriorityType priority, BlockingType blockType) override;
    Status receive(U8* destination,
                   FwSizeType capacity,
                   BlockingType blockType,
                   FwSizeType& actualSize,
                   FwQueuePriorityType& priority) override;

    FwSizeType getMessagesAvailable() const override;
    FwSizeType getMessageHighWaterMark() const override;
    QueueHandle* getHandle() override;

  private:
    void releaseAllocations();

    FreeRtosQueueHandle m_handle;
};

}  // namespace Queue
}  // namespace FreeRTOS
}  // namespace Os

#endif
