#include "ESP32/Drv/Esp32WifiDriver/Esp32WifiDriver.hpp"

#include <cstring>

#if defined(ESP_PLATFORM)
extern "C" {
#include <errno.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lwip/inet.h>
#include <lwip/sockets.h>
}
#endif

namespace Drv {

Esp32WifiDriver::Esp32WifiDriver(const char* compName)
    : Esp32WifiDriverComponentBase(compName),
      m_configured(false),
      m_ready_sent(false),
      m_socket(-1),
      m_remote_port(0),
      m_local_port(0),
      m_rx_buffer_size(2048),
      m_connection_state(ConnectionState::DISCONNECTED) {
#if defined(ESP_PLATFORM)
    this->m_remote_ip[0] = '\0';
    this->m_remote_addr = {};
#endif
}

Esp32WifiDriver::~Esp32WifiDriver() {
    this->closeSocket();
}

bool Esp32WifiDriver::configure(const char* remote_ip, U16 remote_port, U16 local_port, U32 rx_buffer_size) {
    this->closeSocket();
    this->m_rx_buffer_size = rx_buffer_size;
    this->m_remote_port = remote_port;
    this->m_local_port = local_port;

#if defined(ESP_PLATFORM)
    if (remote_ip == nullptr || remote_port == 0U) {
        return false;
    }
    if (std::strlen(remote_ip) >= sizeof(this->m_remote_ip)) {
        return false;
    }
    std::strncpy(this->m_remote_ip, remote_ip, sizeof(this->m_remote_ip));
    this->m_remote_ip[sizeof(this->m_remote_ip) - 1] = '\0';

    this->m_remote_addr = {};
    this->m_remote_addr.sin_family = AF_INET;
    this->m_remote_addr.sin_port = htons(remote_port);
    this->m_remote_addr.sin_addr.s_addr = inet_addr(this->m_remote_ip);
    if (this->m_remote_addr.sin_addr.s_addr == INADDR_NONE) {
        return false;
    }
#else
    static_cast<void>(remote_ip);
    static_cast<void>(remote_port);
    static_cast<void>(local_port);
#endif

    this->m_configured = true;
    this->m_ready_sent = false;
    this->m_connection_state = ConnectionState::DISCONNECTED;
    return true;
}

bool Esp32WifiDriver::pollTransportReady() {
    if (!this->ensureConnected()) {
        return false;
    }
    this->primeReady();
    return this->m_connection_state == ConnectionState::CONNECTED;
}

bool Esp32WifiDriver::openSocket() {
#if defined(ESP_PLATFORM)
    if (this->m_socket >= 0) {
        return this->m_connection_state == ConnectionState::CONNECTED;
    }

    this->m_socket = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (this->m_socket < 0) {
        return false;
    }

    timeval timeout = {};
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    static_cast<void>(lwip_setsockopt(this->m_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)));
    static_cast<void>(lwip_setsockopt(this->m_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)));

    int reuse = 1;
    static_cast<void>(lwip_setsockopt(this->m_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)));

    const int status =
        lwip_connect(this->m_socket, reinterpret_cast<const sockaddr*>(&this->m_remote_addr), sizeof(this->m_remote_addr));
    if (status == 0) {
        this->m_connection_state = ConnectionState::CONNECTED;
        return true;
    }

    this->closeSocket();
    return false;
#else
    return false;
#endif
}

bool Esp32WifiDriver::ensureConnected() {
    if (!this->m_configured) {
        return false;
    }
    if (this->m_connection_state == ConnectionState::CONNECTED) {
        return true;
    }
    return this->openSocket();
}

void Esp32WifiDriver::closeSocket() {
#if defined(ESP_PLATFORM)
    if (this->m_socket >= 0) {
        static_cast<void>(lwip_close(this->m_socket));
    }
#endif
    this->m_socket = -1;
    this->m_ready_sent = false;
    this->m_connection_state = ConnectionState::DISCONNECTED;
}

void Esp32WifiDriver::primeReady() {
    if (!this->m_configured || (this->m_connection_state != ConnectionState::CONNECTED)) {
        return;
    }
    if (!this->m_ready_sent && this->isConnected_ready_OutputPort(0)) {
        this->ready_out(0);
        this->m_ready_sent = true;
    }
}

void Esp32WifiDriver::run_handler(FwIndexType portNum, U32 context) {
    static_cast<void>(portNum);
    static_cast<void>(context);
    if (!this->pollTransportReady()) {
        return;
    }

    if (!this->isConnected_recv_OutputPort(0)) {
        return;
    }

    Fw::Buffer buffer = this->allocate_out(0, this->m_rx_buffer_size);
    if ((buffer.getData() == nullptr) || (buffer.getSize() == 0U)) {
        return;
    }

#if defined(ESP_PLATFORM)
    const int read = lwip_recv(this->m_socket, buffer.getData(), static_cast<size_t>(buffer.getSize()), MSG_DONTWAIT);
    if (read > 0) {
        buffer.setSize(static_cast<FwSizeType>(read));
        this->recv_out(0, buffer, Drv::ByteStreamStatus::OP_OK);
        return;
    }
    if (read == 0) {
        this->closeSocket();
    } else if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
        this->closeSocket();
    }
#endif

    this->deallocate_out(0, buffer);
}

Drv::ByteStreamStatus Esp32WifiDriver::send_handler(FwIndexType portNum, Fw::Buffer& sendBuffer) {
    static_cast<void>(portNum);
#if defined(ESP_PLATFORM)
    if (!this->ensureConnected()) {
        return Drv::ByteStreamStatus::OTHER_ERROR;
    }

    FwSizeType total_sent = 0;
    U32 retryCount = 0;
    static constexpr U32 MAX_SEND_RETRIES = 5;
    while (total_sent < sendBuffer.getSize()) {
        const int sent = lwip_send(
            this->m_socket, sendBuffer.getData() + total_sent, static_cast<size_t>(sendBuffer.getSize() - total_sent), 0);
        if (sent > 0) {
            total_sent += static_cast<FwSizeType>(sent);
            retryCount = 0;
            continue;
        }
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == ENOBUFS)) {
            if (retryCount < MAX_SEND_RETRIES) {
                ++retryCount;
                vTaskDelay(pdMS_TO_TICKS(10));
                continue;
            }
        }
        this->closeSocket();
        return Drv::ByteStreamStatus::OTHER_ERROR;
    }
    return Drv::ByteStreamStatus::OP_OK;
#else
    static_cast<void>(sendBuffer);
    return Drv::ByteStreamStatus::OTHER_ERROR;
#endif
}

void Esp32WifiDriver::recvReturnIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    static_cast<void>(portNum);
    this->deallocate_out(0, fwBuffer);
}

}  // namespace Drv
