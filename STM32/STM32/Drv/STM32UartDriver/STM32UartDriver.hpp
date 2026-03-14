// ======================================================================
// \title  STM32UartDriver.hpp
// \brief  STM32 UART driver skeleton for FreeRTOS targets
// ======================================================================
#ifndef Drv_STM32UartDriver_HPP
#define Drv_STM32UartDriver_HPP

#include <STM32/Drv/STM32UartDriver/STM32UartDriverComponentAc.hpp>
#include <array>
#include <atomic>

namespace Drv {

class STM32UartDriver final : public STM32UartDriverComponentBase {
  public:
    using StartRxDmaFn = bool (*)(void* context, U8* destination, FwSizeType size);
    using StartTxDmaFn = bool (*)(void* context, const U8* source, FwSizeType size);

    explicit STM32UartDriver(const char* const compName);
    ~STM32UartDriver() override;

    STM32UartDriver(const STM32UartDriver& other) = delete;
    STM32UartDriver& operator=(const STM32UartDriver& other) = delete;

    //! Set HAL/BSP hooks for DMA startup without adding HAL compile dependencies here.
    void setDmaHooks(void* context, StartRxDmaFn startRx, StartTxDmaFn startTx);

    //! Start RX DMA and advertise readiness.
    bool start();

    //! ISR hook for UART idle-line notifications. Call from ISR with current DMA write index.
    void onUartIdleIsr(FwSizeType dmaWriteIndex);

    //! ISR hook for TX DMA completion callback.
    void onTxDmaCompleteIsr();

  private:
    static constexpr FwSizeType RX_DMA_RING_SIZE = 1024;
    static constexpr FwSizeType TX_QUEUE_DEPTH = 8;
    static constexpr FwSizeType TX_MAX_FRAME_SIZE = 512;

    struct TxSlot {
        std::array<U8, TX_MAX_FRAME_SIZE> m_bytes{};
        FwSizeType m_size = 0;
        bool m_in_use = false;
    };

    void run_handler(FwIndexType portNum, U32 context) override;
    Drv::ByteStreamStatus send_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) override;
    void recvReturnIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) override;

    void processRxNotification();
    void processTxNotification();
    void kickTxDmaIfIdle();
    FwSizeType getRxAvailable(FwSizeType writeIndex) const;
    void copyFromRxRing(U8* destination, FwSizeType offset, FwSizeType size) const;

    std::array<U8, RX_DMA_RING_SIZE> m_rxDmaRing{};
    std::array<TxSlot, TX_QUEUE_DEPTH> m_txQueue{};

    std::atomic<bool> m_rxNotificationPending;
    std::atomic<bool> m_txCompletionPending;
    std::atomic<bool> m_txDmaBusy;
    std::atomic<FwSizeType> m_latestRxWriteIndex;

    FwSizeType m_rxReadIndex;
    FwSizeType m_txHead;
    FwSizeType m_txTail;
    FwSizeType m_txCount;
    FwSizeType m_txInFlight;
    bool m_txInFlightValid;

    void* m_dmaContext;
    StartRxDmaFn m_startRxDma;
    StartTxDmaFn m_startTxDma;
};

}  // namespace Drv

#endif
