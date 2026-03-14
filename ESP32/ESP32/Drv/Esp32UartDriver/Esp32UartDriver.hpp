#ifndef FPRIME_ESP32_UART_DRIVER_HPP
#define FPRIME_ESP32_UART_DRIVER_HPP

#include "ESP32/Drv/Esp32UartDriver/Esp32UartDriverComponentAc.hpp"

namespace Drv {

class Esp32UartDriver final : public Esp32UartDriverComponentBase {
  public:
    explicit Esp32UartDriver(const char* compName);
    ~Esp32UartDriver() override;

    bool configure(U32 uart_num, U32 baud, U32 rx_buffer_size = 2048, U32 tx_buffer_size = 2048);

  private:
    void run_handler(FwIndexType portNum, U32 context) override;
    Drv::ByteStreamStatus send_handler(FwIndexType portNum, Fw::Buffer& sendBuffer) override;
    void recvReturnIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) override;

    bool m_configured;
    U32 m_uart_num;
    U32 m_rx_buffer_size;
};

}  // namespace Drv

#endif
