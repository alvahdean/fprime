// ======================================================================
// \title  STM32UartDriverTester.hpp
// \brief  test harness for STM32UartDriver
// ======================================================================
#ifndef Drv_STM32UartDriverTester_HPP
#define Drv_STM32UartDriverTester_HPP

#include <STM32/Drv/STM32UartDriver/STM32UartDriver.hpp>
#include <STM32/Drv/STM32UartDriver/STM32UartDriverGTestBase.hpp>
#include <vector>

namespace Drv {

class STM32UartDriverTester final : public STM32UartDriverGTestBase {
  public:
    static constexpr FwSizeType MAX_HISTORY_SIZE = 128;
    static constexpr FwEnumStoreType TEST_INSTANCE_ID = 0;

    STM32UartDriverTester();
    ~STM32UartDriverTester() override;

    void test_start_and_ready();
    void test_rx_deferral_and_ring_wrap();
    void test_tx_queue_and_completion_deferral();
    void test_tx_overflow_and_retry_recovery();

  private:
    struct RxRecord {
        std::vector<U8> bytes;
        Drv::ByteStreamStatus status;
        Fw::Buffer buffer;
    };

    void connectPorts();
    void initComponents();
    void clearState();

    void from_ready_handler(FwIndexType portNum) override;
    void from_recv_handler(FwIndexType portNum, Fw::Buffer& buffer, const Drv::ByteStreamStatus& status) override;
    Fw::Buffer from_allocate_handler(FwIndexType portNum, FwSizeType size) override;
    void from_deallocate_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) override;

    static bool startRxDmaThunk(void* context, U8* destination, FwSizeType size);
    static bool startTxDmaThunk(void* context, const U8* source, FwSizeType size);
    bool startRxDma(U8* destination, FwSizeType size);
    bool startTxDma(const U8* source, FwSizeType size);

    Drv::ByteStreamStatus sendBytes(const std::vector<U8>& bytes);
    void fillRxRing(FwSizeType offset, const std::vector<U8>& bytes);

    STM32UartDriver component;

    U8* m_rxRing;
    FwSizeType m_rxRingSize;
    bool m_rxStarted;

    bool m_txStartShouldFail;
    FwSizeType m_txStartCalls;
    std::vector<std::vector<U8>> m_txStartedFrames;
    std::vector<RxRecord> m_rxRecords;
};

}  // namespace Drv

#endif
