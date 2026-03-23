#ifndef ESP32_DRV_ESP32GPIODRIVER_HPP
#define ESP32_DRV_ESP32GPIODRIVER_HPP

#include "ESP32/Drv/Esp32GpioDriver/Esp32GpioDriverComponentAc.hpp"

namespace Drv {

class Esp32GpioDriver final : public Esp32GpioDriverComponentBase {
  public:
    explicit Esp32GpioDriver(const char* compName);
    ~Esp32GpioDriver() override;

    bool configure(U32 pin, bool activeHigh = true, Fw::Logic defaultState = Fw::Logic::LOW);

  private:
    Drv::GpioStatus gpioRead_handler(FwIndexType portNum, Fw::Logic& state) override;
    Drv::GpioStatus gpioWrite_handler(FwIndexType portNum, const Fw::Logic& state) override;

    bool m_configured;
    U32 m_pin;
    bool m_active_high;
    Fw::Logic m_last_state;
};

}  // namespace Drv

#endif
