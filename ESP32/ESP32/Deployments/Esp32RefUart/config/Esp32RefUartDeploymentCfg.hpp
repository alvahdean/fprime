#ifndef ESP32REFUART_DEPLOYMENT_CFG_HPP
#define ESP32REFUART_DEPLOYMENT_CFG_HPP

#define ESP32_REF_UART_NUM 0
#define ESP32_REF_UART_BAUD 115200
#define ESP32_REF_UART_TX_PIN -1
#define ESP32_REF_UART_RX_PIN -1

// To use the USB/serial connector as the Comm port, use the ESP32 defaults
// uncomment the lines below, note that the USB/Serial port is also used 
// as a console log and so separating the F' comms from the logging is recommended
// To use one of the other 2 UARTs that the ESP32 chip provides
// For example, the Huzzah32 board exposes the UART1 pins TX=17, RX=16
#define ESP32_REF_UART_NUM 1
#define ESP32_REF_UART_BAUD 115200
#define ESP32_REF_UART_TX_PIN 17
#define ESP32_REF_UART_RX_PIN 16

#define ESP32_REF_LED_PIN 13
#define ESP32_REF_LED_ACTIVE_HIGH 1

namespace Esp32RefUartConfig {

static constexpr unsigned int UART_NUM = static_cast<unsigned int>(ESP32_REF_UART_NUM);
static constexpr unsigned int UART_BAUD = static_cast<unsigned int>(ESP32_REF_UART_BAUD);
static constexpr int UART_TX_PIN = ESP32_REF_UART_TX_PIN;
static constexpr int UART_RX_PIN = ESP32_REF_UART_RX_PIN;
static constexpr unsigned int LED_PIN = static_cast<unsigned int>(ESP32_REF_LED_PIN);
static constexpr bool LED_ACTIVE_HIGH = (ESP32_REF_LED_ACTIVE_HIGH != 0);

static_assert(UART_NUM <= 2U, "ESP32_REF_UART_NUM must be in the range [0, 2]");
static_assert(UART_BAUD > 0U, "ESP32_REF_UART_BAUD must be positive");
static_assert(UART_TX_PIN >= -1, "ESP32_REF_UART_TX_PIN must be -1 or a valid GPIO number");
static_assert(UART_RX_PIN >= -1, "ESP32_REF_UART_RX_PIN must be -1 or a valid GPIO number");
static_assert(ESP32_REF_LED_ACTIVE_HIGH == 0 || ESP32_REF_LED_ACTIVE_HIGH == 1,
              "ESP32_REF_LED_ACTIVE_HIGH must be 0 or 1");

}  // namespace Esp32RefUartConfig

#endif
