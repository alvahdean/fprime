#include "ESP32/Drv/Esp32GpioDriver/Esp32GpioDriver.hpp"

#include <cstdio>

#if defined(ESP_PLATFORM)
extern "C" {
#include <driver/gpio.h>
}
#endif

namespace {

int physicalLevel(const Fw::Logic state, const bool activeHigh) {
    const bool logicalHigh = (state == Fw::Logic::HIGH);
    const bool physicalHigh = activeHigh ? logicalHigh : !logicalHigh;
    return physicalHigh ? 1 : 0;
}

Fw::Logic logicalLevel(const int physicalLevelValue, const bool activeHigh) {
    const bool physicalHigh = physicalLevelValue != 0;
    const bool logicalHigh = activeHigh ? physicalHigh : !physicalHigh;
    return logicalHigh ? Fw::Logic::HIGH : Fw::Logic::LOW;
}

}  // namespace

namespace Drv {

Esp32GpioDriver::Esp32GpioDriver(const char* compName)
    : Esp32GpioDriverComponentBase(compName),
      m_configured(false),
      m_pin(0),
      m_active_high(true),
      m_last_state(Fw::Logic::LOW) {}

Esp32GpioDriver::~Esp32GpioDriver() = default;

bool Esp32GpioDriver::configure(U32 pin, bool activeHigh, Fw::Logic defaultState) {
#if defined(ESP_PLATFORM)
    const gpio_num_t gpio_pin = static_cast<gpio_num_t>(pin);
    if (gpio_reset_pin(gpio_pin) != ESP_OK) {
        return false;
    }
    if (gpio_set_direction(gpio_pin, GPIO_MODE_OUTPUT) != ESP_OK) {
        return false;
    }
    if (gpio_set_level(gpio_pin, physicalLevel(defaultState, activeHigh)) != ESP_OK) {
        return false;
    }
#endif
    this->m_pin = pin;
    this->m_active_high = activeHigh;
    this->m_last_state = defaultState;
    this->m_configured = true;
    return true;
}

Drv::GpioStatus Esp32GpioDriver::gpioRead_handler(FwIndexType portNum, Fw::Logic& state) {
    static_cast<void>(portNum);
    if (!this->m_configured) {
        state = this->m_last_state;
        return Drv::GpioStatus::NOT_OPENED;
    }
    state = this->m_last_state;
    return Drv::GpioStatus::OP_OK;
}

Drv::GpioStatus Esp32GpioDriver::gpioWrite_handler(FwIndexType portNum, const Fw::Logic& state) {
    static_cast<void>(portNum);
    if (!this->m_configured) {
        return Drv::GpioStatus::NOT_OPENED;
    }
#if defined(ESP_PLATFORM)
    if (gpio_set_level(static_cast<gpio_num_t>(this->m_pin), physicalLevel(state, this->m_active_high)) != ESP_OK) {
        return Drv::GpioStatus::UNKNOWN_ERROR;
    }
#endif
    this->m_last_state = state;
    return Drv::GpioStatus::OP_OK;
}

}  // namespace Drv
