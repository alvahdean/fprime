// ======================================================================
// \title  STM32UartDriverTester.cpp
// \brief  test harness for STM32UartDriver
// ======================================================================

#include "STM32UartDriverTester.hpp"
#include <cstring>

namespace Drv {

STM32UartDriverTester::STM32UartDriverTester()
    : STM32UartDriverGTestBase("STM32UartDriverTester", STM32UartDriverTester::MAX_HISTORY_SIZE),
      component("STM32UartDriver"),
      m_rxRing(nullptr),
      m_rxRingSize(0),
      m_rxStarted(false),
      m_txStartShouldFail(false),
      m_txStartCalls(0) {
    this->initComponents();
    this->connectPorts();
    this->component.setDmaHooks(this, &STM32UartDriverTester::startRxDmaThunk, &STM32UartDriverTester::startTxDmaThunk);
}

STM32UartDriverTester::~STM32UartDriverTester() {
    for (RxRecord& record : this->m_rxRecords) {
        delete[] record.buffer.getData();
        record.buffer.setData(nullptr);
        record.buffer.setSize(0);
    }
}

void STM32UartDriverTester::test_start_and_ready() {
    this->clearState();
    const bool started = this->component.start();
    ASSERT_TRUE(started);
    ASSERT_TRUE(this->m_rxStarted);
    ASSERT_NE(this->m_rxRing, nullptr);
    ASSERT_GT(this->m_rxRingSize, 0U);
    ASSERT_from_ready_SIZE(1);
}

void STM32UartDriverTester::test_rx_deferral_and_ring_wrap() {
    this->clearState();
    ASSERT_TRUE(this->component.start());

    // No data delivery should occur until run_handler processes deferred ISR notifications.
    const std::vector<U8> first = {0x01, 0x02, 0x03, 0x04};
    this->fillRxRing(0, first);
    this->component.onUartIdleIsr(first.size());
    ASSERT_TRUE(this->m_rxRecords.empty());
    this->invoke_to_run(0, 0);
    ASSERT_EQ(this->m_rxRecords.size(), 1U);
    ASSERT_EQ(this->m_rxRecords[0].status, Drv::ByteStreamStatus::OP_OK);
    ASSERT_EQ(this->m_rxRecords[0].bytes, first);

    // Return first RX buffer ownership to driver and verify deallocation path.
    Fw::Buffer firstReturn = this->m_rxRecords[0].buffer;
    this->invoke_to_recvReturnIn(0, firstReturn);
    this->m_rxRecords[0].buffer.setData(nullptr);
    this->m_rxRecords[0].buffer.setSize(0);
    ASSERT_from_deallocate_SIZE(1);

    // Advance read index near the end of the ring, then verify wrapped copy.
    std::vector<U8> priming(1023, 0xAA);
    this->fillRxRing(0, priming);
    this->component.onUartIdleIsr(1023);
    this->invoke_to_run(0, 0);
    ASSERT_EQ(this->m_rxRecords.size(), 2U);

    const std::vector<U8> wrapped = {0x11, 0x22, 0x33};
    this->fillRxRing(1023, {wrapped[0]});
    this->fillRxRing(0, {wrapped[1], wrapped[2]});
    this->component.onUartIdleIsr(2);
    this->invoke_to_run(0, 0);
    ASSERT_EQ(this->m_rxRecords.size(), 3U);
    ASSERT_EQ(this->m_rxRecords[2].bytes, wrapped);
}

void STM32UartDriverTester::test_tx_queue_and_completion_deferral() {
    this->clearState();
    ASSERT_TRUE(this->component.start());

    const Drv::ByteStreamStatus firstStatus = this->sendBytes({0x10, 0x11, 0x12});
    ASSERT_EQ(firstStatus, Drv::ByteStreamStatus::OP_OK);
    ASSERT_EQ(this->m_txStartCalls, 1U);
    ASSERT_EQ(this->m_txStartedFrames.size(), 1U);

    const Drv::ByteStreamStatus secondStatus = this->sendBytes({0x20, 0x21});
    ASSERT_EQ(secondStatus, Drv::ByteStreamStatus::OP_OK);
    // Second frame is queued until TX completion ISR is serviced in run().
    ASSERT_EQ(this->m_txStartCalls, 1U);

    this->component.onTxDmaCompleteIsr();
    ASSERT_EQ(this->m_txStartCalls, 1U);
    this->invoke_to_run(0, 0);
    ASSERT_EQ(this->m_txStartCalls, 2U);
    ASSERT_EQ(this->m_txStartedFrames.size(), 2U);
    ASSERT_EQ(this->m_txStartedFrames[1], (std::vector<U8>{0x20, 0x21}));
}

void STM32UartDriverTester::test_tx_overflow_and_retry_recovery() {
    this->clearState();
    ASSERT_TRUE(this->component.start());
    this->m_txStartShouldFail = true;

    for (FwSizeType index = 0; index < 8; index++) {
        const Drv::ByteStreamStatus status = this->sendBytes({static_cast<U8>(0x30 + index)});
        ASSERT_EQ(status, Drv::ByteStreamStatus::OP_OK);
    }
    const Drv::ByteStreamStatus overflow = this->sendBytes({0xFF});
    ASSERT_EQ(overflow, Drv::ByteStreamStatus::SEND_RETRY);
    ASSERT_GE(this->m_txStartCalls, 1U);

    // Allow DMA starts, then drive completion interrupts to drain the queue.
    this->m_txStartShouldFail = false;
    for (FwSizeType index = 0; index < 8; index++) {
        this->invoke_to_run(0, 0);
        this->component.onTxDmaCompleteIsr();
    }
    this->invoke_to_run(0, 0);

    // Queue space should be available again and retries can succeed.
    const Drv::ByteStreamStatus retry = this->sendBytes({0xEE});
    ASSERT_EQ(retry, Drv::ByteStreamStatus::OP_OK);
}

void STM32UartDriverTester::clearState() {
    this->clearFromPortHistory();
    this->clearHistory();
    this->m_rxRing = nullptr;
    this->m_rxRingSize = 0;
    this->m_rxStarted = false;
    this->m_txStartShouldFail = false;
    this->m_txStartCalls = 0;
    this->m_txStartedFrames.clear();
    for (RxRecord& record : this->m_rxRecords) {
        delete[] record.buffer.getData();
    }
    this->m_rxRecords.clear();
}

void STM32UartDriverTester::from_ready_handler(FwIndexType portNum) {
    this->pushFromPortEntry_ready();
}

void STM32UartDriverTester::from_recv_handler(FwIndexType portNum, Fw::Buffer& buffer, const Drv::ByteStreamStatus& status) {
    this->pushFromPortEntry_recv(buffer, status);
    RxRecord record;
    record.status = status;
    record.buffer = buffer;
    record.bytes.resize(buffer.getSize());
    if (buffer.getSize() > 0U) {
        std::memcpy(record.bytes.data(), buffer.getData(), buffer.getSize());
    }
    this->m_rxRecords.push_back(record);
}

Fw::Buffer STM32UartDriverTester::from_allocate_handler(FwIndexType portNum, FwSizeType size) {
    this->pushFromPortEntry_allocate(size);
    return Fw::Buffer(new U8[size], size);
}

void STM32UartDriverTester::from_deallocate_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    this->pushFromPortEntry_deallocate(fwBuffer);
    delete[] fwBuffer.getData();
    fwBuffer.setData(nullptr);
    fwBuffer.setSize(0);
}

bool STM32UartDriverTester::startRxDmaThunk(void* context, U8* destination, FwSizeType size) {
    return reinterpret_cast<STM32UartDriverTester*>(context)->startRxDma(destination, size);
}

bool STM32UartDriverTester::startTxDmaThunk(void* context, const U8* source, FwSizeType size) {
    return reinterpret_cast<STM32UartDriverTester*>(context)->startTxDma(source, size);
}

bool STM32UartDriverTester::startRxDma(U8* destination, FwSizeType size) {
    this->m_rxStarted = true;
    this->m_rxRing = destination;
    this->m_rxRingSize = size;
    return true;
}

bool STM32UartDriverTester::startTxDma(const U8* source, FwSizeType size) {
    this->m_txStartCalls++;
    if (this->m_txStartShouldFail) {
        return false;
    }
    std::vector<U8> frame(size);
    if (size > 0U) {
        std::memcpy(frame.data(), source, size);
    }
    this->m_txStartedFrames.push_back(frame);
    return true;
}

Drv::ByteStreamStatus STM32UartDriverTester::sendBytes(const std::vector<U8>& bytes) {
    U8 storage[64] = {};
    FW_ASSERT(bytes.size() <= FW_NUM_ARRAY_ELEMENTS(storage), static_cast<FwAssertArgType>(bytes.size()));
    for (FwSizeType index = 0; index < bytes.size(); index++) {
        storage[index] = bytes[index];
    }
    Fw::Buffer buffer(storage, bytes.size());
    return this->invoke_to_send(0, buffer);
}

void STM32UartDriverTester::fillRxRing(FwSizeType offset, const std::vector<U8>& bytes) {
    FW_ASSERT(this->m_rxRing != nullptr);
    FW_ASSERT(this->m_rxRingSize > 0U);
    for (FwSizeType index = 0; index < bytes.size(); index++) {
        this->m_rxRing[(offset + index) % this->m_rxRingSize] = bytes[index];
    }
}

}  // namespace Drv
