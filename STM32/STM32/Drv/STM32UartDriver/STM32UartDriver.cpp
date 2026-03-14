// ======================================================================
// \title  STM32UartDriver.cpp
// \brief  STM32 UART driver skeleton for FreeRTOS targets
// ======================================================================
#include <STM32/Drv/STM32UartDriver/STM32UartDriver.hpp>

namespace Drv {

STM32UartDriver::STM32UartDriver(const char* const compName)
    : STM32UartDriverComponentBase(compName),
      m_rxNotificationPending(false),
      m_txCompletionPending(false),
      m_txDmaBusy(false),
      m_latestRxWriteIndex(0),
      m_rxReadIndex(0),
      m_txHead(0),
      m_txTail(0),
      m_txCount(0),
      m_txInFlight(0),
      m_txInFlightValid(false),
      m_dmaContext(nullptr),
      m_startRxDma(nullptr),
      m_startTxDma(nullptr) {}

STM32UartDriver::~STM32UartDriver() = default;

void STM32UartDriver::setDmaHooks(void* context, StartRxDmaFn startRx, StartTxDmaFn startTx) {
    this->m_dmaContext = context;
    this->m_startRxDma = startRx;
    this->m_startTxDma = startTx;
}

bool STM32UartDriver::start() {
    bool started = true;
    if (this->m_startRxDma != nullptr) {
        started = this->m_startRxDma(this->m_dmaContext, this->m_rxDmaRing.data(), this->m_rxDmaRing.size());
    }
    if (started && this->isConnected_ready_OutputPort(0)) {
        this->ready_out(0);
    }
    return started;
}

void STM32UartDriver::onUartIdleIsr(const FwSizeType dmaWriteIndex) {
    this->m_latestRxWriteIndex.store(dmaWriteIndex % RX_DMA_RING_SIZE);
    this->m_rxNotificationPending.store(true);
}

void STM32UartDriver::onTxDmaCompleteIsr() {
    this->m_txDmaBusy.store(false);
    this->m_txCompletionPending.store(true);
}

void STM32UartDriver::run_handler(FwIndexType portNum, U32 context) {
    (void)portNum;
    (void)context;
    if (this->m_txCompletionPending.exchange(false)) {
        this->processTxNotification();
    }
    if (this->m_rxNotificationPending.exchange(false)) {
        this->processRxNotification();
    }
    this->kickTxDmaIfIdle();
}

Drv::ByteStreamStatus STM32UartDriver::send_handler(const FwIndexType portNum, Fw::Buffer& fwBuffer) {
    (void)portNum;
    if (fwBuffer.getData() == nullptr || fwBuffer.getSize() == 0) {
        return Drv::ByteStreamStatus::OTHER_ERROR;
    }
    if (fwBuffer.getSize() > TX_MAX_FRAME_SIZE) {
        return Drv::ByteStreamStatus::OTHER_ERROR;
    }
    if (this->m_txCount >= TX_QUEUE_DEPTH) {
        return Drv::ByteStreamStatus::SEND_RETRY;
    }

    TxSlot& slot = this->m_txQueue[this->m_txTail];
    for (FwSizeType index = 0; index < fwBuffer.getSize(); index++) {
        slot.m_bytes[index] = fwBuffer.getData()[index];
    }
    slot.m_size = fwBuffer.getSize();
    slot.m_in_use = true;

    this->m_txTail = (this->m_txTail + 1) % TX_QUEUE_DEPTH;
    this->m_txCount += 1;
    this->kickTxDmaIfIdle();
    return Drv::ByteStreamStatus::OP_OK;
}

void STM32UartDriver::recvReturnIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    (void)portNum;
    if (this->isConnected_deallocate_OutputPort(0)) {
        this->deallocate_out(0, fwBuffer);
    }
}

void STM32UartDriver::processRxNotification() {
    if (!this->isConnected_allocate_OutputPort(0) || !this->isConnected_recv_OutputPort(0)) {
        return;
    }

    const FwSizeType writeIndex = this->m_latestRxWriteIndex.load();
    while (true) {
        const FwSizeType available = this->getRxAvailable(writeIndex);
        if (available == 0) {
            return;
        }
        Fw::Buffer rxBuffer = this->allocate_out(0, available);
        if (rxBuffer.getData() == nullptr || rxBuffer.getSize() == 0) {
            return;
        }
        const FwSizeType emitSize = (rxBuffer.getSize() < available) ? rxBuffer.getSize() : available;
        this->copyFromRxRing(rxBuffer.getData(), this->m_rxReadIndex, emitSize);
        rxBuffer.setSize(emitSize);
        this->m_rxReadIndex = (this->m_rxReadIndex + emitSize) % RX_DMA_RING_SIZE;
        this->recv_out(0, rxBuffer, Drv::ByteStreamStatus::OP_OK);
    }
}

void STM32UartDriver::processTxNotification() {
    if (!this->m_txInFlightValid) {
        return;
    }
    this->m_txQueue[this->m_txInFlight].m_in_use = false;
    this->m_txQueue[this->m_txInFlight].m_size = 0;
    this->m_txInFlightValid = false;
    if (this->m_txCount > 0) {
        this->m_txHead = (this->m_txHead + 1) % TX_QUEUE_DEPTH;
        this->m_txCount -= 1;
    }
}

void STM32UartDriver::kickTxDmaIfIdle() {
    if (this->m_txDmaBusy.load() || this->m_txCount == 0) {
        return;
    }

    TxSlot& slot = this->m_txQueue[this->m_txHead];
    if (!slot.m_in_use || slot.m_size == 0) {
        return;
    }

    if (this->m_startTxDma == nullptr) {
        // No HAL hook installed yet; drop one queued frame to avoid a permanent backlog in skeleton mode.
        slot.m_in_use = false;
        slot.m_size = 0;
        this->m_txHead = (this->m_txHead + 1) % TX_QUEUE_DEPTH;
        this->m_txCount -= 1;
        return;
    }

    const bool started = this->m_startTxDma(this->m_dmaContext, slot.m_bytes.data(), slot.m_size);
    if (started) {
        this->m_txInFlight = this->m_txHead;
        this->m_txInFlightValid = true;
        this->m_txDmaBusy.store(true);
    }
}

FwSizeType STM32UartDriver::getRxAvailable(const FwSizeType writeIndex) const {
    if (writeIndex >= this->m_rxReadIndex) {
        return writeIndex - this->m_rxReadIndex;
    }
    return (RX_DMA_RING_SIZE - this->m_rxReadIndex) + writeIndex;
}

void STM32UartDriver::copyFromRxRing(U8* destination, const FwSizeType offset, const FwSizeType size) const {
    for (FwSizeType index = 0; index < size; index++) {
        destination[index] = this->m_rxDmaRing[(offset + index) % RX_DMA_RING_SIZE];
    }
}

}  // namespace Drv
