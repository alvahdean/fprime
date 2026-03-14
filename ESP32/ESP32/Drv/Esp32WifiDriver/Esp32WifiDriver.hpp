#ifndef FPRIME_ESP32_WIFI_DRIVER_HPP
#define FPRIME_ESP32_WIFI_DRIVER_HPP

#include "ESP32/Drv/Esp32WifiDriver/Esp32WifiDriverComponentAc.hpp"
#if defined(ESP_PLATFORM)
extern "C" {
#include <lwip/sockets.h>
}
#endif

namespace Drv {

class Esp32WifiDriver final : public Esp32WifiDriverComponentBase {
  public:
    explicit Esp32WifiDriver(const char* compName);
    ~Esp32WifiDriver() override;

    bool configure(const char* remote_ip, U16 remote_port, U16 local_port, U32 rx_buffer_size = 2048);

  private:
    void primeReady();
    void run_handler(FwIndexType portNum, U32 context) override;
    Drv::ByteStreamStatus send_handler(FwIndexType portNum, Fw::Buffer& sendBuffer) override;
    void recvReturnIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) override;

    bool m_configured;
    bool m_ready_sent;
    int m_socket;
    U32 m_rx_buffer_size;
#if defined(ESP_PLATFORM)
    sockaddr_in m_remote_addr;
#endif
};

}  // namespace Drv

#endif
