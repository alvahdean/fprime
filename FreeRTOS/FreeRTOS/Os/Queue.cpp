// ======================================================================
// \title  FreeRTOS/Os/Queue.cpp
// \brief  FreeRTOS hybrid priority queue implementation
// ======================================================================

#include "FreeRTOS/Os/Queue.hpp"

#include <Fw/LanguageHelpers.hpp>
#include <Fw/Types/Assert.hpp>
#include <Fw/Types/MemAllocator.hpp>
#include <cstring>

#include "config/MemoryAllocatorTypeEnumAc.hpp"

namespace Os {
namespace FreeRTOS {
namespace Queue {

FwSizeType FreeRtosQueueHandle::findIndex() {
    FW_ASSERT(this->m_depth > 0);
    FwSizeType index = this->m_indices[this->m_start_index % this->m_depth];
    this->m_start_index = (this->m_start_index + 1) % this->m_depth;
    return index;
}

void FreeRtosQueueHandle::returnIndex(FwSizeType index) {
    FW_ASSERT(this->m_depth > 0);
    this->m_indices[this->m_stop_index % this->m_depth] = index;
    this->m_stop_index = (this->m_stop_index + 1) % this->m_depth;
}

void FreeRtosQueueHandle::storeData(FwSizeType index, const U8* source, FwSizeType size) {
    FW_ASSERT(this->m_data != nullptr);
    FW_ASSERT(size <= this->m_max_size);
    FW_ASSERT(index < this->m_depth);
    (void)::memcpy(this->m_data + (index * this->m_max_size), source, static_cast<size_t>(size));
    this->m_sizes[index] = size;
}

void FreeRtosQueueHandle::loadData(FwSizeType index, U8* destination, FwSizeType size) const {
    FW_ASSERT(this->m_data != nullptr);
    FW_ASSERT(size <= this->m_max_size);
    FW_ASSERT(index < this->m_depth);
    (void)::memcpy(destination, this->m_data + (index * this->m_max_size), static_cast<size_t>(size));
}

FreeRtosQueue::~FreeRtosQueue() {
    this->teardown();
}

void FreeRtosQueue::releaseAllocations() {
    Fw::MemAllocator& allocator = Fw::MemAllocatorRegistry::getInstance().getAnAllocator(
        Fw::MemoryAllocation::MemoryAllocatorType::OS_GENERIC_PRIORITY_QUEUE);

    if (this->m_handle.m_data != nullptr) {
        allocator.deallocate(this->m_handle.m_id, this->m_handle.m_data);
    }
    if (this->m_handle.m_indices != nullptr) {
        allocator.deallocate(this->m_handle.m_id, this->m_handle.m_indices);
    }
    if (this->m_handle.m_sizes != nullptr) {
        allocator.deallocate(this->m_handle.m_id, this->m_handle.m_sizes);
    }
    if (this->m_handle.m_heap_pointer != nullptr) {
        this->m_handle.m_heap.teardown();
        allocator.deallocate(this->m_handle.m_id, this->m_handle.m_heap_pointer);
    }

    this->m_handle.m_data = nullptr;
    this->m_handle.m_indices = nullptr;
    this->m_handle.m_sizes = nullptr;
    this->m_handle.m_heap_pointer = nullptr;
    this->m_handle.m_depth = 0;
    this->m_handle.m_max_size = 0;
    this->m_handle.m_start_index = 0;
    this->m_handle.m_stop_index = 0;
    this->m_handle.m_high_water = 0;
}

QueueInterface::Status FreeRtosQueue::create(FwEnumStoreType id,
                                             const Fw::ConstStringBase& name,
                                             FwSizeType depth,
                                             FwSizeType messageSize) {
    (void)name;
    if ((depth == 0) || (messageSize == 0)) {
        return Status::SIZE_MISMATCH;
    }

    Fw::MemAllocator& allocator = Fw::MemAllocatorRegistry::getInstance().getAnAllocator(
        Fw::MemoryAllocation::MemoryAllocatorType::OS_GENERIC_PRIORITY_QUEUE);

    this->m_handle.m_id = id;

    FwSizeType index_bytes = depth * sizeof(FwSizeType);
    void* allocation = allocator.allocate(id, index_bytes, alignof(FwSizeType));
    if (allocation == nullptr) {
        return Status::ALLOCATION_FAILED;
    }
    this->m_handle.m_indices =
        Fw::arrayPlacementNew<FwSizeType>(Fw::ByteArray(static_cast<U8*>(allocation), index_bytes), depth);

    FwSizeType size_bytes = depth * sizeof(FwSizeType);
    allocation = allocator.allocate(id, size_bytes, alignof(FwSizeType));
    if (allocation == nullptr) {
        this->releaseAllocations();
        return Status::ALLOCATION_FAILED;
    }
    this->m_handle.m_sizes =
        Fw::arrayPlacementNew<FwSizeType>(Fw::ByteArray(static_cast<U8*>(allocation), size_bytes), depth);

    FwSizeType data_bytes = depth * messageSize;
    allocation = allocator.allocate(id, data_bytes, alignof(U8));
    if (allocation == nullptr) {
        this->releaseAllocations();
        return Status::ALLOCATION_FAILED;
    }
    this->m_handle.m_data = static_cast<U8*>(allocation);

    FwSizeType heap_size = Types::MaxHeap::ELEMENT_SIZE * depth;
    allocation = allocator.allocate(id, heap_size, Types::MaxHeap::ALIGNMENT);
    if (allocation == nullptr) {
        this->releaseAllocations();
        return Status::ALLOCATION_FAILED;
    }
    this->m_handle.m_heap_pointer = static_cast<U8*>(allocation);
    this->m_handle.m_heap.create(depth, Fw::ByteArray(static_cast<U8*>(allocation), heap_size));

    this->m_handle.m_lock = xSemaphoreCreateMutex();
    this->m_handle.m_slots = xSemaphoreCreateCounting(static_cast<UBaseType_t>(depth), static_cast<UBaseType_t>(depth));
    this->m_handle.m_items = xSemaphoreCreateCounting(static_cast<UBaseType_t>(depth), 0);
    if ((this->m_handle.m_lock == nullptr) || (this->m_handle.m_slots == nullptr) || (this->m_handle.m_items == nullptr)) {
        this->teardown();
        return Status::ALLOCATION_FAILED;
    }

    for (FwSizeType i = 0; i < depth; i++) {
        this->m_handle.m_indices[i] = i;
        this->m_handle.m_sizes[i] = 0;
    }

    this->m_handle.m_depth = depth;
    this->m_handle.m_max_size = messageSize;
    this->m_handle.m_start_index = 0;
    this->m_handle.m_stop_index = 0;
    this->m_handle.m_high_water = 0;

    return Status::OP_OK;
}

void FreeRtosQueue::teardown() {
    if (this->m_handle.m_lock != nullptr) {
        vSemaphoreDelete(this->m_handle.m_lock);
        this->m_handle.m_lock = nullptr;
    }
    if (this->m_handle.m_slots != nullptr) {
        vSemaphoreDelete(this->m_handle.m_slots);
        this->m_handle.m_slots = nullptr;
    }
    if (this->m_handle.m_items != nullptr) {
        vSemaphoreDelete(this->m_handle.m_items);
        this->m_handle.m_items = nullptr;
    }
    this->releaseAllocations();
}

QueueInterface::Status FreeRtosQueue::send(const U8* buffer,
                                           FwSizeType size,
                                           FwQueuePriorityType priority,
                                           BlockingType blockType) {
    if (size > this->m_handle.m_max_size) {
        return Status::SIZE_MISMATCH;
    }

    const TickType_t wait_ticks = (blockType == BlockingType::BLOCKING) ? portMAX_DELAY : 0;
    if (xSemaphoreTake(this->m_handle.m_slots, wait_ticks) != pdTRUE) {
        return (blockType == BlockingType::BLOCKING) ? Status::SEND_ERROR : Status::FULL;
    }

    if (xSemaphoreTake(this->m_handle.m_lock, portMAX_DELAY) != pdTRUE) {
        (void)xSemaphoreGive(this->m_handle.m_slots);
        return Status::SEND_ERROR;
    }

    const FwSizeType index = this->m_handle.findIndex();
    if (!this->m_handle.m_heap.push(priority, index)) {
        (void)xSemaphoreGive(this->m_handle.m_lock);
        (void)xSemaphoreGive(this->m_handle.m_slots);
        return Status::SEND_ERROR;
    }
    this->m_handle.storeData(index, buffer, size);
    this->m_handle.m_high_water = FW_MAX(this->m_handle.m_high_water, this->m_handle.m_heap.getSize());

    (void)xSemaphoreGive(this->m_handle.m_lock);
    (void)xSemaphoreGive(this->m_handle.m_items);
    return Status::OP_OK;
}

QueueInterface::Status FreeRtosQueue::receive(U8* destination,
                                              FwSizeType capacity,
                                              BlockingType blockType,
                                              FwSizeType& actualSize,
                                              FwQueuePriorityType& priority) {
    const TickType_t wait_ticks = (blockType == BlockingType::BLOCKING) ? portMAX_DELAY : 0;
    if (xSemaphoreTake(this->m_handle.m_items, wait_ticks) != pdTRUE) {
        return (blockType == BlockingType::BLOCKING) ? Status::RECEIVE_ERROR : Status::EMPTY;
    }

    if (xSemaphoreTake(this->m_handle.m_lock, portMAX_DELAY) != pdTRUE) {
        (void)xSemaphoreGive(this->m_handle.m_items);
        return Status::RECEIVE_ERROR;
    }

    FwSizeType index = 0;
    if (!this->m_handle.m_heap.pop(priority, index)) {
        (void)xSemaphoreGive(this->m_handle.m_lock);
        (void)xSemaphoreGive(this->m_handle.m_items);
        return Status::RECEIVE_ERROR;
    }

    actualSize = this->m_handle.m_sizes[index];
    if (actualSize > capacity) {
        this->m_handle.returnIndex(index);
        (void)xSemaphoreGive(this->m_handle.m_lock);
        (void)xSemaphoreGive(this->m_handle.m_slots);
        return Status::SIZE_MISMATCH;
    }

    this->m_handle.loadData(index, destination, actualSize);
    this->m_handle.returnIndex(index);

    (void)xSemaphoreGive(this->m_handle.m_lock);
    (void)xSemaphoreGive(this->m_handle.m_slots);
    return Status::OP_OK;
}

FwSizeType FreeRtosQueue::getMessagesAvailable() const {
    return this->m_handle.m_heap.getSize();
}

FwSizeType FreeRtosQueue::getMessageHighWaterMark() const {
    return this->m_handle.m_high_water;
}

QueueHandle* FreeRtosQueue::getHandle() {
    return &this->m_handle;
}

}  // namespace Queue
}  // namespace FreeRTOS
}  // namespace Os
