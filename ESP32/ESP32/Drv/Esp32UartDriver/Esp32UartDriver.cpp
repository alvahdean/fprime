#include "ESP32/Drv/Esp32UartDriver/Esp32UartDriver.hpp"

#if defined(ESP_PLATFORM)
extern "C" {
#include <driver/uart.h>
}
#endif

namespace Drv {

Esp32UartDriver::Esp32UartDriver(const char* compName)
    : Esp32UartDriverComponentBase(compName), m_configured(false), m_uart_num(0), m_rx_buffer_size(2048) {}

Esp32UartDriver::~Esp32UartDriver() {}

bool Esp32UartDriver::configure(U32 uart_num,
                                U32 baud,
                                U32 rx_buffer_size,
                                U32 tx_buffer_size,
                                I32 tx_pin,
                                I32 rx_pin) {
    this->m_uart_num = uart_num;
    this->m_rx_buffer_size = rx_buffer_size;

#if defined(ESP_PLATFORM)
    uart_config_t config = {};
    config.baud_rate = static_cast<int>(baud);
    config.data_bits = UART_DATA_8_BITS;
    config.parity = UART_PARITY_DISABLE;
    config.stop_bits = UART_STOP_BITS_1;
    config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    config.source_clk = UART_SCLK_DEFAULT;

    if (uart_param_config(static_cast<uart_port_t>(uart_num), &config) != ESP_OK) {
        return false;
    }

    const int configured_tx_pin = (tx_pin >= 0) ? static_cast<int>(tx_pin) : UART_PIN_NO_CHANGE;
    const int configured_rx_pin = (rx_pin >= 0) ? static_cast<int>(rx_pin) : UART_PIN_NO_CHANGE;
    if ((configured_tx_pin != UART_PIN_NO_CHANGE) || (configured_rx_pin != UART_PIN_NO_CHANGE)) {
        if (uart_set_pin(static_cast<uart_port_t>(uart_num),
                         configured_tx_pin,
                         configured_rx_pin,
                         UART_PIN_NO_CHANGE,
                         UART_PIN_NO_CHANGE) != ESP_OK) {
            return false;
        }
    }

    if (uart_driver_install(static_cast<uart_port_t>(uart_num),
                            static_cast<int>(rx_buffer_size),
                            static_cast<int>(tx_buffer_size),
                            0,
                            nullptr,
                            0) != ESP_OK) {
        return false;
    }
#endif

    this->m_configured = true;
    if (this->isConnected_ready_OutputPort(0)) {
        this->ready_out(0);
    }
    return true;
}

void Esp32UartDriver::run_handler(FwIndexType portNum, U32 context) {
    static_cast<void>(portNum);
    static_cast<void>(context);

    if (!this->m_configured || !this->isConnected_recv_OutputPort(0)) {
        return;
    }

    Fw::Buffer buffer = this->allocate_out(0, this->m_rx_buffer_size);
    if ((buffer.getData() == nullptr) || (buffer.getSize() == 0U)) {
        return;
    }
    Drv::ByteStreamStatus status = Drv::ByteStreamStatus::RECV_NO_DATA;

#if defined(ESP_PLATFORM)
    const int read = uart_read_bytes(static_cast<uart_port_t>(this->m_uart_num),
                                     buffer.getData(),
                                     static_cast<uint32_t>(buffer.getSize()),
                                     0);
    if (read > 0) {
        buffer.setSize(static_cast<FwSizeType>(read));
        status = Drv::ByteStreamStatus::OP_OK;
    } else {
        status = Drv::ByteStreamStatus::RECV_NO_DATA;
    }
#else
    status = Drv::ByteStreamStatus::OTHER_ERROR;
#endif

    if (status == Drv::ByteStreamStatus::OP_OK) {
        this->recv_out(0, buffer, status);
    } else {
        this->deallocate_out(0, buffer);
    }
}

Drv::ByteStreamStatus Esp32UartDriver::send_handler(FwIndexType portNum, Fw::Buffer& sendBuffer) {
    static_cast<void>(portNum);

    if (!this->m_configured) {
        return Drv::ByteStreamStatus::OTHER_ERROR;
    }

#if defined(ESP_PLATFORM)
    const int written = uart_write_bytes(static_cast<uart_port_t>(this->m_uart_num),
                                         reinterpret_cast<const char*>(sendBuffer.getData()),
                                         static_cast<size_t>(sendBuffer.getSize()));
    return (written == static_cast<int>(sendBuffer.getSize())) ? Drv::ByteStreamStatus::OP_OK
                                                                : Drv::ByteStreamStatus::OTHER_ERROR;
#else
    return Drv::ByteStreamStatus::OTHER_ERROR;
#endif
}

void Esp32UartDriver::recvReturnIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    static_cast<void>(portNum);
    this->deallocate_out(0, fwBuffer);
}

}  // namespace Drv
