// ======================================================================
// \title  STM32UartDriverTestMain.cpp
// \brief  unit test main for STM32UartDriver
// ======================================================================

#include "STM32UartDriverTester.hpp"

TEST(STM32UartDriver, StartAndReady) {
    Drv::STM32UartDriverTester tester;
    tester.test_start_and_ready();
}

TEST(STM32UartDriver, RxDeferralAndWrap) {
    Drv::STM32UartDriverTester tester;
    tester.test_rx_deferral_and_ring_wrap();
}

TEST(STM32UartDriver, TxQueueAndCompletionDeferral) {
    Drv::STM32UartDriverTester tester;
    tester.test_tx_queue_and_completion_deferral();
}

TEST(STM32UartDriver, TxOverflowRetry) {
    Drv::STM32UartDriverTester tester;
    tester.test_tx_overflow_and_retry_recovery();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
